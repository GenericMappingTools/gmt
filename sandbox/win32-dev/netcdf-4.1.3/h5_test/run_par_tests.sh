#!/bin/sh

# This shell runs some parallel tests.

# $Id$

# Even for successful runs, mpiexec seems to set a non-zero return
# code!  
#set -e
echo ""
echo "Testing parallel I/O with HDF5..."

mpiexec -n 1 ./tst_h_par
mpiexec -n 2 ./tst_h_par
mpiexec -n 4 ./tst_h_par
echo "SUCCESS!!!"

exit 0
