#!/usr/bin/env bash
#
# Test the C API for reading a matrix and test GMT_DUPLICATE_RESET.

gmt grdmath -R1/4/1/3 -I1 X Y MUL = test.grd
testapi_matrix_pad > fail
