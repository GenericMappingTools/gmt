#!/bin/sh
#
#	$Id: time_testing_3.sh,v 1.5 2007-11-15 04:20:42 remko Exp $
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Next we use unix which uses seconds.  Hence the output should cover the
# first 5 seconds in year 1970:

. ../functions.sh
header "Test time conversions (rel time & Unix)"

sample1d -I0.5 << EOF > $$.d
0	0
4	1
EOF
cat << EOF > $$.answer
1970-01-01T00:00:00.0
1970-01-01T00:00:00.5
1970-01-01T00:00:01.0
1970-01-01T00:00:01.5
1970-01-01T00:00:02.0
1970-01-01T00:00:02.5
1970-01-01T00:00:03.0
1970-01-01T00:00:03.5
1970-01-01T00:00:04.0
EOF
gmtconvert $$.d -fi0t -fo0T --TIME_SYSTEM=unix --OUTPUT_CLOCK_FORMAT=hh:mm:ss.x > $$.result

paste $$.result $$.answer | awk '{if ($1 != $3) print $0}' > fail

rm -f $$.*

passfail time_testing_3
