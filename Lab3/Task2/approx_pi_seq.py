#!/usr/bin/env python3

import subprocess
from pathlib import Path

K = 31
ITER = 5

SOURCE_FILE = Path("approx_pi_seq.c")
BUILD_DIR = Path("build")
EXECUTABLE = BUILD_DIR / "approx_pi_seq"


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


def run_program() -> None:
    cmd = [
        str(EXECUTABLE),
        str(K),
    ]

    subprocess.run(
        cmd,
        check=True,
    )


def main() -> None:
    build()

    for _ in range(ITER):
        run_program()

if __name__ == "__main__":
    main()