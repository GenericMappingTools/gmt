#!/usr/bin/env bash
# Test makecpt with time array lacking an increment value

cat << EOF > cpt-true.cpt
2012-06-29T08:00:00	0/0/127	2012-08-27T20:22:30	blue	L
2012-08-27T20:22:30	blue	2012-12-24T21:07:30	cyan	L
2012-12-24T21:07:30	cyan	2013-04-22T21:52:30	yellow	L
2013-04-22T21:52:30	yellow	2013-08-19T22:37:30	red	L
2013-08-19T22:37:30	red	2013-10-18T11:00:00	127/0/0	B
EOF

gmt makecpt -Cjet -T2012-06-29T08:00:00/2013-10-18T11:00:00 -N > cpt-test.cpt

diff cpt-true.cpt cpt-test.cpt --strip-trailing-cr >> fail
