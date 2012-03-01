#!/bin/sh

# This shell runs some parallel I/O tests for the F90 API.

set -e
echo "Testing netCDF parallel I/O through the F90 API."
mpiexec -n 4 ./f90tst_parallel
mpiexec -n 4 ./f90tst_parallel2
mpiexec -n 4 ./f90tst_parallel3
mpiexec -n 8 ./f90tst_nc4_par
#mpiexec -n 4 ./f90tst_parallel_fill

echo "SUCCESS!!!"
exit 0


