#!/bin/bash
#	$Id$
#
# Test the C API for conversions between user vectors and
# datasets.  A user input "dataset" is passed as a vector to
# gmtmath which multiplies by 10 and adds 1, then writes
# out the dataset to another user vector.  We try all possible
# combinations of vector types (char, int, etc) and do
# the case of user pre-allocated output vector or letting
# GMT allocate them.

rm -f fail
testapi_uservectors | grep ":FAIL" > fail
