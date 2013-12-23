#!/bin/bash
#	$Id$
#
# Test to make sure the -Ei option works as advertised.
# We have data that will fall in to 4 separate blocks
# in a -R0/2/0/2 -I2 -r situation (2x2 blocks)
# In all blocks mean = median = mode = 5.  However, we
# are returning the record numbers that go with those
# median/modes, and compare to given answers.

log=rectest.log

cat << EOF > data.d
# Block NW (1 value)
0.4	1.35	5	A
# Block NE (2 values)
1.3	1.7	4	B
1.7	1.3	6	C
# Block SW (5 values)
0.2	0.2	3	D
0.4	0.4	4	E
0.6	0.6	5	F
0.5	0.5	6	G
0.2	0.7	7	H
# Block SE (6 values)
1.3	0.2	2	I
1.4	0.4	3	J
1.7	0.6	4	K
1.5	0.6	6	L
1.2	0.8	7	M
1.9	0.18	8	N
EOF
cat << EOF > truth.d
1
2
6
11
1
3
6
12
1
2
5
10
1
3
7
13
EOF
# Record numbers should match truth.d"
gmt blockmedian -R0/2/0/2 -I1 -Er- -r data.d -o3 > $log
gmt blockmedian -R0/2/0/2 -I1 -Er+ -r data.d -o3 >> $log
gmt blockmode   -R0/2/0/2 -I1 -Er- -r data.d -o3 >> $log
gmt blockmode   -R0/2/0/2 -I1 -Er+ -r data.d -o3 >> $log
diff $log truth.d --strip-trailing-cr > fail
