#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
cd "$SCRIPT_DIR"

CXX=${CXX:-g++}
CXXFLAGS=${CXXFLAGS:-"-std=c++14 -O2 -Wall -Wextra -pedantic"}
REPEATS=${REPEATS:-5}
RESULTS_DIR=${RESULTS_DIR:-results}
CSV_PATH="${RESULTS_DIR}/benchmark.csv"

mkdir -p "$RESULTS_DIR"

"$CXX" $CXXFLAGS benchmark.cpp -o benchmark.out

cat > "$CSV_PATH" <<'CSV'
case_name,scenario,patterns,total_pattern_length,max_pattern_length,text_tokens,inserted_matches,seed,repeats,aho_build_raw_us,aho_search_raw_us,aho_total_raw_us,aho_build_median_us,aho_search_median_us,aho_total_median_us,aho_matches,naive_enabled,naive_raw_us,naive_median_us,naive_matches,aho_vs_naive
CSV

run_case() {
    local case_name=$1
    local scenario=$2
    local patterns=$3
    local pattern_len=$4
    local text_tokens=$5
    local inserted=$6
    local seed=$7
    local naive=$8

    echo "[benchmark] case=${case_name} scenario=${scenario} patterns=${patterns} text=${text_tokens}"

    python3 gen_benchmark_data.py \
        --scenario "$scenario" \
        --patterns "$patterns" \
        --pattern-len "$pattern_len" \
        --text "$text_tokens" \
        --inserted "$inserted" \
        --seed "$seed" \
    | ./benchmark.out \
        --case-name "$case_name" \
        --scenario "$scenario" \
        --inserted "$inserted" \
        --seed "$seed" \
        --repeats "$REPEATS" \
        --naive "$naive" \
    >> "$CSV_PATH"
}

seed=100

for patterns in 10 50 100; do
    run_case "baseline-small-${patterns}" "random" "$patterns" 5 20000 200 "$seed" "on"
    seed=$((seed + 1))
done

for text_tokens in 50000 100000 500000 1000000; do
    inserted=$((text_tokens / 1000))
    run_case "text-scale-${text_tokens}" "random" 500 5 "$text_tokens" "$inserted" "$seed" "off"
    seed=$((seed + 1))
done

for patterns in 100 500 1000 5000; do
    run_case "pattern-scale-${patterns}" "random" "$patterns" 5 200000 500 "$seed" "off"
    seed=$((seed + 1))
done

for max_length in 5 10 20; do
    run_case "overlap-output-${max_length}" "overlap" "$max_length" "$max_length" 100000 0 "$seed" "off"
    seed=$((seed + 1))
done

echo "[benchmark] results: ${CSV_PATH}"
