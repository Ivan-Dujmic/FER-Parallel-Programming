import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

# Input files
seq_file = "measurements/approx_pi_seq.txt"
opencl_file = "measurements/approx_pi.txt"
vec_file = "measurements/approx_pi_vec.txt"
guvec_file = "measurements/approx_pi_guvec.txt"

# Output folder
out_dir = Path("measurements/graphs")
out_dir.mkdir(exist_ok=True)

# Read sequential baseline
# k time_us
seq = pd.read_csv(
    seq_file,
    sep=r"\s+",
    header=None,
    names=["k", "time_seq"]
)

# Aggregate sequential baseline by k
seq = seq.groupby("k", as_index=False)["time_seq"].mean()

# Read parallel results
# NUM_BLOCKS SIZE_BLOCK k atomic? seq time_full_us time_calc_us
par = pd.read_csv(
    opencl_file,
    sep=r"\s+",
    header=None,
    names=[
        "NUM_BLOCKS",
        "SIZE_BLOCK",
        "k",
        "time_par",
    ]
)

par = par.groupby(["NUM_BLOCKS", "SIZE_BLOCK", "k"], as_index=False)["time_par"].mean()

# Merge with sequential baseline by k
df_par = par.merge(seq, on="k", how="inner")

# Factor:
# > 1 means faster than sequential
# < 1 means slower than sequential
df_par["speedup_factor"] = df_par["time_seq"] / df_par["time_par"]

# Read Numba vectorized results
# k time_us
vec = pd.read_csv(
    vec_file,
    sep=r"\s+",
    header=None,
    names=["k", "time_vec"]
)

vec = vec.groupby("k", as_index=False)["time_vec"].mean()

df_vec = vec.merge(seq, on="k", how="inner")
df_vec["speedup_factor"] = df_vec["time_seq"] / df_vec["time_vec"]

# Read Numba ug vectorized results
# k block_size time_us
guvec = pd.read_csv(
    guvec_file,
    sep=r"\s+",
    header=None,
    names=["k", "block_size", "time_guvec"]
)

guvec = guvec.groupby(["k", "block_size"], as_index=False)["time_guvec"].mean()

df_guvec = guvec.merge(seq, on="k", how="inner")
df_guvec["speedup_factor"] = df_guvec["time_seq"] / df_guvec["time_guvec"]

df_guvec_vec = guvec.merge(vec[["k", "time_vec"]], on="k", how="inner")
df_guvec_vec["speedup_factor_vec"] = df_guvec_vec["time_vec"] / df_guvec_vec["time_guvec"]

for k, group in df_par.groupby("k"):
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
    plt.title(f"OpenCL vs Sequential for k={k}")

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

    output_path = out_dir / f"speedup_opencl_k_{k}.png"
    plt.savefig(output_path, dpi=200)
    plt.close()

for k, group in df_vec.groupby("k"):
    print(f"Numba Vectorized vs Sequential for k={k}: {group['speedup_factor'].mean():.2f}x")

for k, group in df_guvec.groupby("k"):
    plt.figure(figsize=(10, 6))
    plt.plot(group["block_size"], group["speedup_factor"], marker="o")
    plt.xscale("log", base=2)
    plt.xlabel("Block Size")
    plt.ylabel("Speedup Factor")
    plt.title(f"Numba UG Vectorized vs Sequential for k={k}")
    plt.grid(True, which="both", ls="--", lw=0.5)
    plt.tight_layout()

    output_path = out_dir / f"speedup_guvec_k_{k}.png"
    plt.savefig(output_path, dpi=200)
    plt.close()

for k, group in df_guvec_vec.groupby("k"):
    plt.figure(figsize=(10, 6))
    plt.plot(group["block_size"], group["speedup_factor_vec"], marker="o")
    plt.xscale("log", base=2)
    plt.xlabel("Block Size")
    plt.ylabel("Speedup Factor (UG Vectorized vs Vectorized)")
    plt.title(f"Numba UG Vectorized vs Vectorized for k={k}")
    plt.grid(True, which="both", ls="--", lw=0.5)
    plt.tight_layout()

    output_path = out_dir / f"speedup_guvec_vs_vec_k_{k}.png"
    plt.savefig(output_path, dpi=200)
    plt.close()

print("Generated graphs:")
for path in sorted(out_dir.glob("*.png")):
    print(path)