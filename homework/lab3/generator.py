#!/usr/bin/env python3

import argparse
import random
import string


class KeyPool:
    def __init__(self):
        self.keys = []
        self.positions = {}

    def add(self, key):
        if key in self.positions:
            return
        self.positions[key] = len(self.keys)
        self.keys.append(key)

    def remove(self, key):
        index = self.positions.pop(key)
        last_key = self.keys.pop()
        if index < len(self.keys):
            self.keys[index] = last_key
            self.positions[last_key] = index

    def random_key(self, rng):
        return self.keys[rng.randrange(len(self.keys))]

    def __contains__(self, key):
        return key in self.positions

    def __len__(self):
        return len(self.keys)


SCENARIO_WEIGHTS = {
    "mixed": (("find", 50), ("insert", 25), ("erase", 25)),
    "read-heavy": (("find", 80), ("insert", 10), ("erase", 10)),
    "update-heavy": (("insert", 40), ("erase", 40), ("find", 20)),
}


def parse_args():
    parser = argparse.ArgumentParser(description="Generate benchmark workload for RB-tree dictionary")
    parser.add_argument("--scenario", choices=sorted(SCENARIO_WEIGHTS.keys()), required=True)
    parser.add_argument("--ops", type=int, required=True, help="number of measured operations")
    parser.add_argument("--prefill", type=int, required=True, help="number of warmup insertions")
    parser.add_argument("--seed", type=int, default=1)
    return parser.parse_args()


def random_base_key(rng):
    length = rng.randint(8, 24)
    return "".join(rng.choice(string.ascii_lowercase) for _ in range(length))


def random_case_variant(rng, key):
    return "".join(char.upper() if rng.random() < 0.5 else char for char in key)


def random_value(rng):
    return rng.getrandbits(64)


def new_absent_key(rng, active_keys):
    while True:
        candidate = random_base_key(rng)
        if candidate not in active_keys:
            return candidate


def choose_weighted_operation(rng, scenario):
    weights = SCENARIO_WEIGHTS[scenario]
    threshold = rng.uniform(0, sum(weight for _, weight in weights))
    current = 0.0

    for operation, weight in weights:
        current += weight
        if threshold <= current:
            return operation

    return weights[-1][0]


def emit_insert(rng, active_keys, allow_duplicate):
    if allow_duplicate and len(active_keys) > 0 and rng.random() < 0.15:
        key = active_keys.random_key(rng)
        print(f"+ {random_case_variant(rng, key)} {random_value(rng)}")
        return

    key = new_absent_key(rng, active_keys)
    active_keys.add(key)
    print(f"+ {random_case_variant(rng, key)} {random_value(rng)}")


def emit_find(rng, active_keys):
    if len(active_keys) > 0 and rng.random() < 0.80:
        key = active_keys.random_key(rng)
    else:
        key = new_absent_key(rng, active_keys)
    print(random_case_variant(rng, key))


def emit_erase(rng, active_keys, require_existing=False):
    if len(active_keys) > 0 and (require_existing or rng.random() < 0.80):
        key = active_keys.random_key(rng)
        print(f"- {random_case_variant(rng, key)}")
        active_keys.remove(key)
        return

    key = new_absent_key(rng, active_keys)
    print(f"- {random_case_variant(rng, key)}")


def main():
    args = parse_args()
    if args.ops <= 0:
        raise SystemExit("--ops must be positive")
    if args.prefill < 0:
        raise SystemExit("--prefill must be non-negative")

    rng = random.Random(args.seed)
    active_keys = KeyPool()

    for _ in range(args.prefill):
        key = new_absent_key(rng, active_keys)
        active_keys.add(key)
        print(f"+ {random_case_variant(rng, key)} {random_value(rng)}")

    lower_bound = args.prefill // 2 if args.prefill > 0 else None
    upper_bound = (3 * args.prefill) // 2 if args.prefill > 0 else None

    for _ in range(args.ops):
        forced_operation = None

        if args.prefill > 0 and len(active_keys) < max(1, lower_bound):
            forced_operation = "insert"
        elif args.prefill > 0 and len(active_keys) > max(1, upper_bound):
            forced_operation = "erase-existing"

        operation = forced_operation or choose_weighted_operation(rng, args.scenario)

        if operation != "insert" and len(active_keys) == 0:
            operation = "insert"

        if operation == "insert":
            emit_insert(rng, active_keys, allow_duplicate=True)
        elif operation == "find":
            emit_find(rng, active_keys)
        elif operation == "erase":
            emit_erase(rng, active_keys)
        else:
            emit_erase(rng, active_keys, require_existing=True)


if __name__ == "__main__":
    main()
