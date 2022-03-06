#!/usr/bin/env bash
#
# Test the GMT_Put_Vector function for mixed textcolumns of
# Cartesian values, datetime, longitude, and latitude strings

# This file holds what is expected to be produced on output
cat << EOF > answer.txt
134.9	2020-06-01T14:55:33	-12.76	-16.25
-158	2020-06-01T16:00:51	19.8	29.5
EOF
testapi_putvector > results.txt
diff -q --strip-trailing-cr results.txt answer.txt > fail
