#!/bin/sh

# This shell runs some parallel I/O tests for the F77 API.

set -e
echo "Testing netCDF parallel I/O through the F77 API..."

mpiexec -n 4 ./ftst_parallel
mpiexec -n 4 ./ftst_parallel_nasa

echo "SUCCESS!!!"
exit 0
