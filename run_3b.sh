#!/bin/bash
for p in A B C D E; do ./cmake-build-debug/application scenarios/3b/$p.csv & sleep 0.03; done
wait
