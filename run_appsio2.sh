#!/bin/bash
# Cenário 4 do Trabalho 1 (só relevante para MLFQ): mais processos e maior
# contenção de CPU, com bloqueios de E/S. Arrancar primeiro o simulador:
#   ./cmake-build-debug/ossim --sched MLFQ

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP="${1:-$HERE/cmake-build-debug/application}"
DIR="$HERE/scenarios/4"

for name in A B C; do
    "$APP" "$DIR/$name.csv" &
done

wait
