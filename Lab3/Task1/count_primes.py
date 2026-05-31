#!/usr/bin/env python3

import itertools
import subprocess
from pathlib import Path
import re
import os

NUM_BLOCKS_VALUES = [32, 64, 128, 256, 512]
SIZE_BLOCK_VALUES = [16, 32, 64, 128, 256]
K_VALUES = [20, 22, 24]
ATOMIC_VALUES = ["t", "f"]

SOURCE_FILE = Path("count_primes.c")
BUILD_DIR = Path("build")
EXECUTABLE = BUILD_DIR / "count_primes"


def patch_defines(num_blocks: int, size_block: int) -> None:
    text = SOURCE_FILE.read_text()

    text = replace_define(text, "NUM_BLOCKS", num_blocks)
    text = replace_define(text, "SIZE_BLOCK", size_block)

    SOURCE_FILE.write_text(text)


def replace_define(text: str, name: str, value: int) -> str:
    pattern = rf"^\s*#define\s+{name}\s+\d+\s*$"
    replacement = f"#define {name} {value}"

    new_text, count = re.subn(pattern, replacement, text, flags=re.MULTILINE)

    if count != 1:
        raise RuntimeError(f"Expected exactly one #define for {name}, found {count}")

    return new_text


def build() -> None:
    BUILD_DIR.mkdir(exist_ok=True)

    subprocess.run(
        [
            "gcc",
            str(SOURCE_FILE),
            "-o",
            str(EXECUTABLE),
            "-lOpenCL",
        ],
        check=True,
    )


def run_program(k: int, atomic: str) -> None:
    cmd = [
        str(EXECUTABLE),
        str(k),
        atomic,
        "seq",
    ]

    env = {
        **os.environ,
        "RUSTICL_ENABLE": "radeonsi",
    }

    subprocess.run(
        cmd,
        check=True,
        env=env,
    )


def main() -> None:
    for num_blocks, size_block in itertools.product(NUM_BLOCKS_VALUES, SIZE_BLOCK_VALUES):
        patch_defines(num_blocks, size_block)
        build()

        for k, atomic in itertools.product(K_VALUES, ATOMIC_VALUES):
            run_program(k, atomic)

if __name__ == "__main__":
    main()