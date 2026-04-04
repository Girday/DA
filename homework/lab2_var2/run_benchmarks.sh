#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
cd "$SCRIPT_DIR"

OUTPUT_CSV=${1:-benchmark_results.csv}
REPEATS=${REPEATS:-5}
SCENARIOS=${SCENARIOS:-"mixed read-heavy update-heavy"}
OPS_LIST=${OPS_LIST:-"50000 100000 200000 500000 1000000"}
APPEND=${APPEND:-0}

if [[ "$APPEND" != "1" ]]; then
    rm -f "$OUTPUT_CSV"
fi

seed=1

for scenario in $SCENARIOS; do
    for ops in $OPS_LIST; do
        prefill=$(( ops / 10 ))
        if (( prefill < 5000 )); then
            prefill=5000
        fi

        echo "[benchmark] scenario=${scenario} ops=${ops} prefill=${prefill} repeats=${REPEATS} seed=${seed}"
        python3 generator.py \
            --scenario "$scenario" \
            --ops "$ops" \
            --prefill "$prefill" \
            --seed "$seed" \
        | ./benchmark.out \
            --scenario "$scenario" \
            --repeats "$REPEATS" \
            --prefill "$prefill" \
            --csv "$OUTPUT_CSV"

        seed=$(( seed + 1 ))
    done
done

echo "[benchmark] results written to ${OUTPUT_CSV}"
