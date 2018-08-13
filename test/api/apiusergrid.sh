#!/bin/bash
#
# Test the C API for conversions between user matrices and
# grids.  A user input "grid" is passed as a matrix to
# grdmath which multiplies by 10 and adds 1, then writes
# out the grid to another user matrix.  We try all possible
# combinations of matrix types (char, intc, etc) and do
# the case of user pre-allocated output matrix or letting
# GMT allocate it.

testapi_usergrid  -q > fail
