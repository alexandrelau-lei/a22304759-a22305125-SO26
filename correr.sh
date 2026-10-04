#!/bin/bash
ALG=$1
SCRIPT=$2
NAME=${SCRIPT%.sh}
mkdir -p logs
./cmake-build-debug/ossim --sched $ALG > logs/${ALG}_${NAME}_sim.txt 2>&1 &
SIM=$!
sleep 1
./$SCRIPT > logs/${ALG}_${NAME}_apps.txt 2>&1
kill -TERM $SIM
wait $SIM 2>/dev/null
grep finished logs/${ALG}_${NAME}_apps.txt
