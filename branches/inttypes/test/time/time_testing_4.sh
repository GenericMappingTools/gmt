#!/bin/bash
#
#	$Id$
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Finally we use a custom epoch that uses days since 1969-07-21T02:56:00.
# Hence the output should cover the first five 24-hour periods following
# the historic Apollo 11 moon-landing; these 24-hour periods crosses
# normal day boundaries:

sample1d -I0.5 << EOF > tt.d
0	0
4	1
EOF
cat << EOF > tt.answer
1969-07-21T02:56:00	0
1969-07-21T14:56:00	0.125
1969-07-22T02:56:00	0.25
1969-07-22T14:56:00	0.375
1969-07-23T02:56:00	0.5
1969-07-23T14:56:00	0.625
1969-07-24T02:56:00	0.75
1969-07-24T14:56:00	0.875
1969-07-25T02:56:00	1
EOF
gmtconvert tt.d -fi0t -fo0T --TIME_EPOCH=1969-07-21T02:56:00 --TIME_UNIT=d > tt.result

diff tt.result tt.answer --strip-trailing-cr > fail
