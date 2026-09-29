#!/bin/bash
# Cenário 1 do Trabalho 1: três aplicações só com burst de CPU (A=10s, B=15s, C=20s).
# Arrancar primeiro o simulador (./build/ossim [--sched ...]) noutro terminal.

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP="${1:-$HERE/build/application}"
DIR="$HERE/scenarios/1"

"$APP" "$DIR/A.csv" &
"$APP" "$DIR/B.csv" &
"$APP" "$DIR/C.csv" &

wait
