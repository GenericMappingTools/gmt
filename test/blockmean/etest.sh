#!/bin/sh
#	$Id: etest.sh,v 1.1 2006-04-05 01:58:00 pwessel Exp $
#
# Test to make sure the -E option works as advertised.
# We have data that will fall in to 4 separate blocks
# in a -R0/2/0/2 -I2 -F situation (2x2 blocks)
# In all blocks mean = median = mode = 5.
cat << EOF > data.d
# Block NW (1 value)
0.4	1.35	5
# Block NE (2 values)
1.3	1.7	4
1.7	1.3	6
# Block SW (5 values)
0.2	0.2	3
0.4	0.4	4
0.6	0.6	5
0.5	0.5	6
0.2	0.7	7
# Block SE (6 values)
1.3	0.2	2
1.4	0.4	3
1.7	0.6	4
1.5	0.6	6
1.2	0.8	7
1.9	0.18	8
EOF
echo "Plain means"
blockmean -R0/2/0/2 -I1 -F data.d
echo "Extended means"
blockmean -R0/2/0/2 -I1 -F -E data.d
echo "Plain medians"
blockmedian -R0/2/0/2 -I1 -F data.d
echo "Extended medians"
blockmedian -R0/2/0/2 -I1 -F -E data.d
echo "Plain modes"
blockmode -R0/2/0/2 -I1 -F data.d
echo "Extended modes"
blockmode -R0/2/0/2 -I1 -F -E data.d
rm -f data.d
