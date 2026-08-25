#!/usr/bin/env bash
#
# Test that passing a user-create GMT_GRID object to GMT API
#
# This file holds what is expected to be produced on output
cat << EOF > answer.txt
0	3	20
1	3	21
2	3	22
3	3	23
4	3	24
0	2	29
1	2	30
2	2	31
3	2	32
4	2	33
0	1	38
1	1	39
2	1	40
3	1	41
4	1	42
0	0	47
1	0	48
2	0	49
3	0	50
4	0	51
EOF
testapi_gmtgrid > results.txt
diff -q --strip-trailing-cr results.txt answer.txt > fail
