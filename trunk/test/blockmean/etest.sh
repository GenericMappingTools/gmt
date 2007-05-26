#!/bin/sh
#	$Id: etest.sh,v 1.3 2007-05-26 02:56:52 pwessel Exp $
#
# Test to make sure the -E option works as advertised.
# We have data that will fall in to 4 separate blocks
# in a -R0/2/0/2 -I2 -F situation (2x2 blocks)
# In all blocks mean = median = mode = 5.

echo -n "GMT: Test blockmean's new -E option on given data:		"

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
echo "All the mean/median/mode values should be 5" > log
echo "Plain means" >> log
blockmean -R0/2/0/2 -I1 -F data.d >> log
echo "Extended means" >> log
blockmean -R0/2/0/2 -I1 -F -E data.d >> log
echo "Plain medians" >> log
blockmedian -R0/2/0/2 -I1 -F data.d >> log
echo "Extended medians" >> log
blockmedian -R0/2/0/2 -I1 -F -E data.d >> log
echo "Plain modes" >> log
blockmode -R0/2/0/2 -I1 -F data.d >> log
echo "Extended modes" >> log
blockmode -R0/2/0/2 -I1 -F -E data.d >> log
awk '{if (NF == 6 && $3 != 5) print $0}' log > fail
if [ -s fail ]; then
	echo "[FAILED]"
else
	echo "[OK"]
	rm -f log
fi
rm -f data.d fail
