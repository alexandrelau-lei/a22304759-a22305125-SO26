#!/bin/bash
# Arranca uma ou mais aplicações (application) contra um simulador (ossim) que
# JÁ esteja em execução noutro terminal. Este script NÃO arranca o simulador.
#
# Uso:
#   ./run_app.sh <ficheiro.csv> [<ficheiro.csv> ...]
#   ./run_app.sh -n <N> <ficheiro.csv> [<ficheiro.csv> ...]   # N cópias de cada ficheiro
#
# Exemplos:
#   ./run_app.sh scenarios/1/A.csv scenarios/1/B.csv scenarios/1/C.csv
#   ./run_app.sh -n 3 A-5-2.csv
#
# Binário: ./cmake-build-debug/application (ou a variável de ambiente $APP, se definida).

set -u

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP="${APP:-$HERE/cmake-build-debug/application}"

count=1
if [ "${1:-}" = "-n" ]; then
    count="${2:-}"
    shift 2 || { echo "Uso: $0 [-n N] <ficheiro.csv> [...]" >&2; exit 1; }
fi

if [ "$#" -eq 0 ]; then
    echo "Uso: $0 [-n N] <ficheiro.csv> [<ficheiro.csv> ...]" >&2
    exit 1
fi

if [ ! -x "$APP" ]; then
    echo "Erro: executável '$APP' não encontrado. Compilar primeiro:  cmake --build build" >&2
    exit 1
fi

pids=()
for csv in "$@"; do
    if [ ! -f "$csv" ]; then
        echo "Aviso: ficheiro '$csv' não existe — ignorado." >&2
        continue
    fi
    i=0
    while [ "$i" -lt "$count" ]; do
        "$APP" "$csv" &
        pids+=($!)
        i=$((i + 1))
    done
done

if [ "${#pids[@]}" -eq 0 ]; then
    echo "Nada para executar." >&2
    exit 1
fi

# Espera que todas as aplicações terminem.
status=0
for p in "${pids[@]}"; do
    wait "$p" || status=1
done
exit "$status"
