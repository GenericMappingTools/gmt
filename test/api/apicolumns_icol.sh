#!/usr/bin/env bash
#
# Test the -i option for external vectors or matrices passed as datasets

# This file holds what is expected to be produced on output
gmt set FORMAT_FLOAT_OUT %8.3f
gmt convert @wus_gps_final_crowell.txt -i1,0,3,3+d2,2,5+s10,6+o10  > answer.txt
testapi_columns > results.txt
diff -q --strip-trailing-cr results.txt answer.txt > fail
