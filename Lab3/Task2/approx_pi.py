#!/usr/bin/env python3

import itertools
import subprocess
from pathlib import Path
import re
import os

NUM_BLOCKS_VALUES = [32, 64, 128, 256, 512]
SIZE_BLOCK_VALUES = [16, 32, 64, 128, 256]
K = 31
ITER = 5

SOURCE_FILE = Path("approx_pi.c")
BUILD_DIR = Path("build")
EXECUTABLE = BUILD_DIR / "approx_pi"


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


def run_program() -> None:
    cmd = [
        str(EXECUTABLE),
        str(K),
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
        for _ in range(ITER):
            run_program()

if __name__ == "__main__":
    main()