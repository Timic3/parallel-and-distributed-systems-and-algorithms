#!/bin/bash

#SBATCH --job-name=first-job
#SBATCH --ntasks=4
#SBATCH --nodes=1
#SBATCH --reservation=fri
#SBATCH --mem-per-cpu=100MB
#SBATCH --output=job.out
#SBATCH --time=00:01:00

srun hostname