#!/bin/bash
# Cenário 3 do Trabalho 1 (só relevante para MLFQ): aplicações com burst de CPU
# e períodos de bloqueio (E/S). Arrancar primeiro o simulador noutro terminal:
#   ./cmake-build-debug/ossim --sched MLFQ

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP="${1:-$HERE/cmake-build-debug/application}"
DIR="$HERE/scenarios/3"

for name in A B C; do
    "$APP" "$DIR/$name.csv" &
done

wait
