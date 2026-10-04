#!/bin/bash
for p in A B C; do ./cmake-build-debug/application scenarios/mlfq2/$p.csv & sleep 0.03; done
wait
