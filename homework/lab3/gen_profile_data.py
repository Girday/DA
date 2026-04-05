#!/usr/bin/env python3

import pathlib
import random
import string
import sys


def random_word(rng: random.Random, min_len: int = 8, max_len: int = 24) -> str:
    length = rng.randint(min_len, max_len)
    return "".join(rng.choice(string.ascii_letters) for _ in range(length))


def main() -> int:
    if len(sys.argv) < 3:
        print(f"usage: {sys.argv[0]} <output> <size> [seed]", file=sys.stderr)
        return 1

    output = pathlib.Path(sys.argv[1])
    size = int(sys.argv[2])
    seed = int(sys.argv[3]) if len(sys.argv) > 3 else 1

    rng = random.Random(seed)
    used: set[str] = set()

    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="ascii") as fh:
        while len(used) < size:
            word = random_word(rng)
            lowered = word.lower()
            if lowered in used:
                continue
            used.add(lowered)
            fh.write(f"{word} {rng.getrandbits(64)}\n")

    print(f"generated {size} entries in {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
