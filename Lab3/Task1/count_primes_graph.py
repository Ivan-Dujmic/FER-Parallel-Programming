import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

# Input files
seq_file = "measurements/count_primes_seq.txt"
parallel_file = "measurements/count_primes.txt"

# Output folder
out_dir = Path("measurements/graphs")
out_dir.mkdir(exist_ok=True)

# Read sequential baseline
# k seq time_us
seq = pd.read_csv(
    seq_file,
    sep=r"\s+",
    header=None,
    names=["k", "label", "seq_time"]
)

seq = seq[["k", "seq_time"]]

# Read parallel results
# NUM_BLOCKS SIZE_BLOCK k atomic? seq time_full_us time_calc_us
par = pd.read_csv(
    parallel_file,
    sep=r"\s+",
    header=None,
    names=[
        "NUM_BLOCKS",
        "SIZE_BLOCK",
        "k",
        "enabled",
        "label",
        "time_full",
        "time_calc",
    ]
)

# Keep only atomic=true rows
par = par[par["enabled"] == "t"].copy()

# Merge with sequential baseline by k
df = par.merge(seq, on="k", how="inner")

# Factor:
# > 1 means faster than sequential
# < 1 means slower than sequential
df["speedup_factor"] = df["seq_time"] / df["time_full"]

for k, group in df.groupby("k"):
    plt.figure(figsize=(10, 6))

    scatter = plt.scatter(
        group["NUM_BLOCKS"],
        group["SIZE_BLOCK"],
        s=0,
        edgecolors="black",
        facecolors="none"
    )

    plt.xscale("log", base=2)
    plt.yscale("log", base=2)

    plt.xlabel("NUM_BLOCKS")
    plt.ylabel("SIZE_BLOCK")
    plt.title(f"Speedup vs Sequential for k={k}")

    # speedup_factor = 1 means equal to sequential
    for _, row in group.iterrows():
        plt.text(
            row["NUM_BLOCKS"],
            row["SIZE_BLOCK"],
            f"{row['speedup_factor']:.2f}x",
            ha="center",
            va="center",
            fontsize=16,
        )

    plt.tight_layout()

    output_path = out_dir / f"speedup_k_{k}.png"
    plt.savefig(output_path, dpi=200)
    plt.close()

print("Generated graphs:")
for path in sorted(out_dir.glob("*.png")):
    print(path)