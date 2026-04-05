#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
cd "$SCRIPT_DIR"

PROFILE_SIZE=${PROFILE_SIZE:-200000}
PROFILE_SEED=${PROFILE_SEED:-1}
PROFILE_RUNS=${PROFILE_RUNS:-5}
BENCHMARK_REPEATS=${BENCHMARK_REPEATS:-5}
BENCHMARK_OPS=${BENCHMARK_OPS:-200000}
BENCHMARK_PREFILL=${BENCHMARK_PREFILL:-20000}
BENCHMARK_SCENARIOS=${BENCHMARK_SCENARIOS:-"mixed read-heavy update-heavy"}
ENABLE_SAMPLE=${ENABLE_SAMPLE:-0}

mkdir -p data results

PROFILE_DATA="data/profile_${PROFILE_SIZE}.txt"
PROFILE_CSV="results/profile_times.csv"
BENCHMARK_CSV="results/benchmark_compare.csv"

rm -f "$PROFILE_CSV" "$BENCHMARK_CSV"

python3 gen_profile_data.py "$PROFILE_DATA" "$PROFILE_SIZE" "$PROFILE_SEED"

for binary in profile_baseline.out profile_fixed.out; do
    for spec in "insert 5" "find 10" "erase 5" "save 10" "load 10"; do
        mode=${spec%% *}
        iterations=${spec##* }
        echo "[profile] binary=${binary} mode=${mode}"
        "./${binary}" \
            --mode "$mode" \
            --dataset "$PROFILE_DATA" \
            --runs "$PROFILE_RUNS" \
            --iterations "$iterations" \
            --tmpfile "results/${binary%.out}_${mode}.bin" \
            --csv "$PROFILE_CSV" \
        | tee "results/${binary%.out}_${mode}.txt"
    done
done

seed=100
for scenario in $BENCHMARK_SCENARIOS; do
    workload_path="results/workload_${scenario}_${BENCHMARK_OPS}.txt"
    python3 generator.py \
        --scenario "$scenario" \
        --ops "$BENCHMARK_OPS" \
        --prefill "$BENCHMARK_PREFILL" \
        --seed "$seed" \
    > "$workload_path"

    for binary in benchmark_baseline.out benchmark_fixed.out; do
        echo "[benchmark] binary=${binary} scenario=${scenario}"
        "./${binary}" \
            --scenario "$scenario" \
            --repeats "$BENCHMARK_REPEATS" \
            --prefill "$BENCHMARK_PREFILL" \
            --csv "$BENCHMARK_CSV" \
        < "$workload_path" \
        | tee "results/${binary%.out}_${scenario}.txt"
    done

    seed=$(( seed + 1 ))
done

echo "[memory] leaks baseline load"
leaks --atExit -- ./profile_baseline.out --mode load --dataset "$PROFILE_DATA" --runs 1 --iterations 1 --tmpfile results/leaks_baseline_load.bin > results/leaks_baseline_load.txt 2>&1 || true
echo "[memory] leaks fixed load"
leaks --atExit -- ./profile_fixed.out --mode load --dataset "$PROFILE_DATA" --runs 1 --iterations 1 --tmpfile results/leaks_fixed_load.bin > results/leaks_fixed_load.txt 2>&1 || true

echo "[memory] heap baseline load"
./profile_baseline.out --mode load --dataset "$PROFILE_DATA" --runs 1 --iterations 1 --hold-ms 4000 --tmpfile results/heap_baseline_load.bin > results/heap_baseline_load_run.txt 2>&1 &
baseline_heap_pid=$!
sleep 1
heap -H "$baseline_heap_pid" > results/heap_baseline_load.txt 2>&1 || true
wait "$baseline_heap_pid" || true

echo "[memory] heap fixed load"
./profile_fixed.out --mode load --dataset "$PROFILE_DATA" --runs 1 --iterations 1 --hold-ms 4000 --tmpfile results/heap_fixed_load.bin > results/heap_fixed_load_run.txt 2>&1 &
fixed_heap_pid=$!
sleep 1
heap -H "$fixed_heap_pid" > results/heap_fixed_load.txt 2>&1 || true
wait "$fixed_heap_pid" || true

if [[ "$ENABLE_SAMPLE" == "1" ]]; then
    echo "[sample] baseline load"
    ./profile_baseline.out --mode load --dataset "$PROFILE_DATA" --runs 1 --iterations 30 --tmpfile results/sample_baseline_load.bin > results/sample_baseline_load_run.txt 2>&1 &
    baseline_sample_pid=$!
    sleep 1
    sample "$baseline_sample_pid" 2 1 -mayDie > results/sample_baseline_load.txt 2> results/sample_baseline_load.stderr || true
    wait "$baseline_sample_pid" || true

    echo "[sample] baseline save"
    ./profile_baseline.out --mode save --dataset "$PROFILE_DATA" --runs 1 --iterations 120 --tmpfile results/sample_baseline_save.bin > results/sample_baseline_save_run.txt 2>&1 &
    baseline_save_pid=$!
    sleep 1
    sample "$baseline_save_pid" 2 1 -mayDie > results/sample_baseline_save.txt 2> results/sample_baseline_save.stderr || true
    wait "$baseline_save_pid" || true
fi

echo "[quality] profile results: ${PROFILE_CSV}"
echo "[quality] benchmark results: ${BENCHMARK_CSV}"
