#!/usr/bin/env bash
#
# Test the C API for conversions between user matrices and
# datasets.  A user input "dataset" is passed as a matrix to
# gmtmath which multiplies by 10 and adds 1, then writes
# out the dataset to another user matrix.  We try all possible
# combinations of matrix types (char, intc, etc) and do
# the case of user pre-allocated output matrix or letting
# GMT allocate it.

testapi_userdataset -q > fail
