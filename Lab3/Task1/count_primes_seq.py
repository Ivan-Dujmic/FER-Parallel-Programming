#!/usr/bin/env python3

import subprocess
from pathlib import Path
import os

K_VALUES = [20, 22, 24]

SOURCE_FILE = Path("count_primes_seq.c")
BUILD_DIR = Path("build")
EXECUTABLE = BUILD_DIR / "count_primes_seq"


def build() -> None:
    BUILD_DIR.mkdir(exist_ok=True)

    subprocess.run(
        [
            "gcc",
            str(SOURCE_FILE),
            "-o",
            str(EXECUTABLE),
            "-lm",
            "-O3"
        ],
        check=True,
    )


def run_program(k: int) -> None:
    cmd = [
        str(EXECUTABLE),
        str(k),
        "seq",
    ]

    subprocess.run(
        cmd,
        check=True,
    )


def main() -> None:
    build()

    for k in K_VALUES:
        run_program(k)

if __name__ == "__main__":
    main()