#!/bin/bash
# Cenário 2 do Trabalho 1: seis aplicações só com burst de CPU
# (A=5s, B=10s, C=4s, D=2s, E=3s, F=15s).
# Arrancar primeiro o simulador (./build/ossim [--sched ...]) noutro terminal.

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP="${1:-$HERE/build/application}"
DIR="$HERE/scenarios/2"

for name in A B C D E F; do
    "$APP" "$DIR/$name.csv" &
done

wait
