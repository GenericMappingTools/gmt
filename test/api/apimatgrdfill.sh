#!/usr/bin/env bash
#
# Test the C API for passing matrices to grdfill with asymmetric pads
echo "output.nc	-54	-50	-23	-16	0	886	1	1	4	7	1	1" > answer.txt

testapi_matrix_grdfill
gmt grdinfo -C output.nc > output.txt
diff answer.txt output.txt > fail
