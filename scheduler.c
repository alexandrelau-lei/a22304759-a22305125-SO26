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
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            printf("Time [ms]: %d\tPID: %d\tDONE\n", current_time_ms, (*cpu_task)->pid);
            finish_burst(current_time_ms, cq, *cpu_task);
            *cpu_task = NULL;
        } else if (sched_algo == SCHED_RR &&
                   current_time_ms - (*cpu_task)->slice_start_ms >= TIME_SLICE_MS) {
            printf("Time [ms]: %d\tPID: %d\tPREEMPTED (slice esgotado)\n",
                   current_time_ms, (*cpu_task)->pid);
            enqueue_pcb(rq, *cpu_task);   // volta ao fim da fila
            *cpu_task = NULL;
                   }
    }

    if (*cpu_task == NULL) {
        if (sched_algo == SCHED_FIFO || sched_algo == SCHED_RR) {
            *cpu_task = dequeue_pcb(rq);
        } else if (sched_algo == SCHED_SJF) {
            *cpu_task = dequeue_pcb_sjf(rq);
        } else {
            printf("Scheduling algorithm not implemented yet.\n");
        }
        if (*cpu_task) {
            (*cpu_task)->slice_start_ms = current_time_ms;
            printf("Time [ms]: %d\tPID: %d\tSTART_RUNNING\n", current_time_ms, (*cpu_task)->pid);
            return 1;
        }
    }
    return 0;
}
