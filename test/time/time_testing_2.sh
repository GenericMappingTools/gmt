#!/bin/sh
#
#	$Id: time_testing_2.sh,v 1.5 2007-11-15 04:20:42 remko Exp $
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Test 2:
# Generate relative time values from 0-4 and use the --TIME_* options to
# convert to absolute calendar time for ascii output.
# 
# First we use J2000 which uses days.  Hence the output should cover the
# first 5 days in year 2000, starting at noon:

. ../functions.sh
header "Test time conversions (rel time & j2000)"

cat << EOF > $$.answer
2000-01-01T12:00:00
2000-01-02T00:00:00
2000-01-02T12:00:00
2000-01-03T00:00:00
2000-01-03T12:00:00
2000-01-04T00:00:00
2000-01-04T12:00:00
2000-01-05T00:00:00
2000-01-05T12:00:00
EOF

sample1d -I0.5 << EOF > $$.d
0	0
4	1
EOF
gmtconvert $$.d -fi0t -fo0T --TIME_SYSTEM=j2000 > $$.result
paste $$.result $$.answer | awk '{if ($1 != $3) print $0}' > fail

rm -f $$.*

passfail time_testing_2
