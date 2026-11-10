#!/bin/bash
#SBATCH --job-name=assignment5
#SBATCH --output=assignment5_%j.out
#SBATCH --error=assignment5_%j.err
#SBATCH --partition=Centaurus
#SBATCH --time=00:30:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --mem=8G


echo "=== Job info ==="
date
echo "User: $USER"
echo "Running on: $(hostname)"
echo "SLURM_JOB_ID: $SLURM_JOB_ID"
echo "Workdir: $(pwd)"
echo
echo "=== Modules ==="
module load gcc
echo
echo "=== Run ==="
make sweep CSV_OUT="benchmark_${SLURM_JOB_ID}.csv"
echo
echo "=== Done ==="
date
