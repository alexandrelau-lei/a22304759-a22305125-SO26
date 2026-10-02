#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "msg.h"

static const char *SCHED_NAMES[] = { "FIFO", "SJF", "RR", "MLFQ", NULL };
static sched_algo_en sched_algo = SCHED_FIFO;

int set_sched_algo(const char *name) {
    for (int i = 0; SCHED_NAMES[i] != NULL; i++) {
        if (strcasecmp(SCHED_NAMES[i], name) == 0) {
            sched_algo = (sched_algo_en) i;
            return sched_algo;
        }
    }
    return -1;
}

const char *get_sched_algo_str(void) {
    return SCHED_NAMES[sched_algo];
}

/**
 * Send DONE to the application and move the finished burst to the command queue.
 */
static void finish_burst(uint32_t current_time_ms, queue_t *cq, pcb_t *task) {
    msg_t msg = {
        .pid = task->pid,
        .request = PROCESS_REQUEST_DONE,
        .time_ms = current_time_ms
    };
    if (write(task->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
        perror("write");
    }
    enqueue_pcb(cq, task);
}


static int warmup_state = 0;   // 0 = à espera do 1.º processo, 1 = a contar, 2 = pronto
static uint32_t warmup_t0;

static int warmup_ok(uint32_t now, queue_t *rq) {
    if (warmup_state == 2) return 1;
    if (warmup_state == 0) {
        if (rq->size == 0) return 0;
        warmup_state = 1;
        warmup_t0 = now;
    }
    if (now - warmup_t0 >= 200) { warmup_state = 2; return 1; }
    return 0;
}

#define MLFQ_LEVELS 3

static uint32_t mlfq_slice(int level) {
    return TIME_SLICE_MS << level;   // 500, 1000, 2000 ms
}

// Devolve o elemento da ready queue com menor nível (o 1.º em caso de empate)
static queue_elem_t *mlfq_pick(queue_t *rq) {
    queue_elem_t *best = NULL;
    for (queue_elem_t *it = rq->head; it != NULL; it = it->next) {
        if (best == NULL || it->pcb->level < best->pcb->level)
            best = it;
    }
    return best;
}

/**
 * FIFO (First-In, First-Out) — não preemptivo.
 *
 * O primeiro processo a chegar à ready queue corre até o seu burst de CPU
 * terminar; só então o CPU fica livre para o processo seguinte.
 *
 * TODO (alunos): acrescentar os algoritmos SJF, RR (time-slice = TIME_SLICE_MS)
 * e MLFQ, selecionados por `sched_algo` / `--sched`.
 */
int scheduler(uint32_t current_time_ms, queue_t *rq, queue_t *cq, pcb_t **cpu_task) {
    if (*cpu_task) {
        pcb_t *t = *cpu_task;
        t->ellapsed_time_ms += TICKS_MS;

        if (t->ellapsed_time_ms >= t->time_ms) {
            if (sched_algo == SCHED_MLFQ) {
                if (!t->slice_full && t->level > 0) {
                    printf("Time [ms]: %d\tPID: %d\tPROMOTED: nivel %d -> %d\n",
                           current_time_ms, t->pid, t->level, t->level - 1);
                    t->level--;
                }
                t->slice_full = 0;
            }
            printf("Time [ms]: %d\tPID: %d\tDONE\n", current_time_ms, t->pid);
            finish_burst(current_time_ms, cq, t);
            *cpu_task = NULL;

        } else if (sched_algo == SCHED_RR &&
                   current_time_ms - t->slice_start_ms >= TIME_SLICE_MS) {
            if (rq->size > 0) {
                printf("Time [ms]: %d\tPID: %d\tPREEMPTED (slice esgotado)\n",
                       current_time_ms, t->pid);
                enqueue_pcb(rq, t);
                *cpu_task = NULL;
            } else {
                t->slice_start_ms = current_time_ms;
            }

        } else if (sched_algo == SCHED_MLFQ) {
            if (current_time_ms - t->slice_start_ms >= mlfq_slice(t->level)) {
                // esgotou o slice: comportamento batch, desce um nível
                t->slice_full = 1;
                int can_demote = t->level < MLFQ_LEVELS - 1;
                if (can_demote) {
                    printf("Time [ms]: %d\tPID: %d\tDEMOTED: nivel %d -> %d (slice esgotado)\n",
                           current_time_ms, t->pid, t->level, t->level + 1);
                    t->level++;
                }
                if (can_demote || rq->size > 0) {
                    enqueue_pcb(rq, t);
                    *cpu_task = NULL;
                } else {
                    t->slice_start_ms = current_time_ms;   // sozinho no nível mais baixo
                }
            } else {
                // preempção por processo de nível mais alto
                queue_elem_t *b = mlfq_pick(rq);
                if (b && b->pcb->level < t->level) {
                    printf("Time [ms]: %d\tPID: %d\tPREEMPTED por PID %d (nivel %d < %d)\n",
                           current_time_ms, t->pid, b->pcb->pid, b->pcb->level, t->level);
                    enqueue_pcb(rq, t);
                    *cpu_task = NULL;
                }
            }
        }
    }

    if (*cpu_task == NULL && warmup_ok(current_time_ms, rq)) {
        if (sched_algo == SCHED_FIFO || sched_algo == SCHED_RR) {
            *cpu_task = dequeue_pcb(rq);
        } else if (sched_algo == SCHED_SJF) {
            *cpu_task = dequeue_pcb_sjf(rq);
        } else if (sched_algo == SCHED_MLFQ) {
            queue_elem_t *e = mlfq_pick(rq);
            if (e) {
                remove_queue_elem(rq, e);
                *cpu_task = e->pcb;
                free(e);
            }
        }
        if (*cpu_task) {
            (*cpu_task)->slice_start_ms = current_time_ms;
            if (!(*cpu_task)->started) {
                (*cpu_task)->started = 1;
                printf("Time [ms]: %d\tPID: %d\tRESPONSE_TIME: %d ms\n", current_time_ms,
                       (*cpu_task)->pid, current_time_ms - (*cpu_task)->arrival_ms);
            }
            if (sched_algo == SCHED_MLFQ)
                printf("Time [ms]: %d\tPID: %d\tSTART_RUNNING (nivel %d)\n",
                       current_time_ms, (*cpu_task)->pid, (*cpu_task)->level);
            else
                printf("Time [ms]: %d\tPID: %d\tSTART_RUNNING\n", current_time_ms, (*cpu_task)->pid);
            return 1;
        }
    }
    return 0;
}
