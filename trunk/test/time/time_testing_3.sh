#!/bin/bash
#
#	$Id$
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Next we use unix which uses seconds.  Hence the output should cover the
# first 5 seconds in year 1970:

gmt sample1d -I0.5 << EOF > tt3.d
0	0
4	1
EOF
cat << EOF > tt3.answer
1970-01-01T00:00:00.0	0
1970-01-01T00:00:00.5	0.125
1970-01-01T00:00:01.0	0.25
1970-01-01T00:00:01.5	0.375
1970-01-01T00:00:02.0	0.5
1970-01-01T00:00:02.5	0.625
1970-01-01T00:00:03.0	0.75
1970-01-01T00:00:03.5	0.875
1970-01-01T00:00:04.0	1
EOF
gmt gmtconvert tt3.d -fi0t -fo0T --TIME_SYSTEM=unix --FORMAT_CLOCK_OUT=hh:mm:ss.x > tt3.result

diff tt3.result tt3.answer --strip-trailing-cr > fail
