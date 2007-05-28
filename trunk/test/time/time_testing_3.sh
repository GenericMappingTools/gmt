#!/bin/sh
#
#	$Id: time_testing_3.sh,v 1.1 2007-05-28 19:40:30 pwessel Exp $
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Next we use unix which uses seconds.  Hence the output should cover the
# first 5 seconds in year 1970:

echo -n "GMT: Test time conversions, part 3 (relative time & Unix):		"

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

paste $$.result $$.answer | awk '{if ($1 != $3) print $0}' > log

if [ -s log ]; then
        echo "[FAILED]"
else
        echo "[OK"]
        rm -f log
fi
rm -f $$.*
