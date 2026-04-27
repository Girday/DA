#!/usr/bin/env python3

import argparse
import random
import sys


BACKGROUND_MAX = 999_999
PATTERN_BASE = 1_000_000_000
LINE_WIDTH = 100


def parse_args():
    parser = argparse.ArgumentParser(description="Generate benchmark input for Aho-Corasick lab4")
    parser.add_argument("--scenario", choices=("random", "overlap"), required=True)
    parser.add_argument("--patterns", type=int, required=True)
    parser.add_argument("--pattern-len", type=int, required=True)
    parser.add_argument("--text", type=int, required=True)
    parser.add_argument("--inserted", type=int, default=0)
    parser.add_argument("--seed", type=int, default=1)
    return parser.parse_args()


def print_sequence(sequence):
    print(" ".join(str(value) for value in sequence))


def print_text(text):
    for start in range(0, len(text), LINE_WIDTH):
        print_sequence(text[start:start + LINE_WIDTH])


def generate_random(args):
    if args.patterns <= 0:
        raise SystemExit("--patterns must be positive")
    if args.pattern_len <= 0:
        raise SystemExit("--pattern-len must be positive")
    if args.text < args.pattern_len and args.inserted > 0:
        raise SystemExit("--text must be at least --pattern-len when --inserted is positive")

    rng = random.Random(args.seed)
    patterns = []

    for pattern_index in range(args.patterns):
        first = PATTERN_BASE + pattern_index * (args.pattern_len + 1)
        patterns.append([first + offset for offset in range(args.pattern_len)])

    text = [rng.randint(0, BACKGROUND_MAX) for _ in range(args.text)]

    max_insertions = args.text // args.pattern_len
    if args.inserted > max_insertions:
        raise SystemExit(f"--inserted is too large for non-overlapping insertions: max is {max_insertions}")

    starts = list(range(0, args.text - args.pattern_len + 1, args.pattern_len))
    rng.shuffle(starts)

    for insert_index, start in enumerate(starts[:args.inserted]):
        pattern = patterns[insert_index % len(patterns)]
        text[start:start + args.pattern_len] = pattern

    for pattern in patterns:
        print_sequence(pattern)
    print()
    print_text(text)


def generate_overlap(args):
    if args.patterns <= 0:
        raise SystemExit("--patterns must be positive")
    if args.text <= 0:
        raise SystemExit("--text must be positive")

    value = PATTERN_BASE

    for length in range(1, args.patterns + 1):
        print_sequence([value] * length)
    print()
    print_text([value] * args.text)


def main():
    args = parse_args()

    if args.scenario == "random":
        generate_random(args)
    else:
        generate_overlap(args)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
