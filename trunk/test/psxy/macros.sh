#!/bin/bash
# Make a seismicity symbol that does r, g, or b depending on depth
# and picks a circle that scales with magnitude range
# For testing, we place a small black triangle on top except
# for the case of z - 50, mag > 9 where we place a white triangle.
# We also draw a large cyan square around intermediate-depth quakes
# i.e., depths in 70-300 km range.

. ./functions.sh
header "Test psxy for multi-parameter custom symbols"

cat << EOF > seis.def
# Circle of variable size and color for seismicity
# Expects 2 extra data columns: \$1 = depth and \$2 = magnitude
N: 2
# First set current color based on depth
if \$1 <= 70 then T -Gred
if \$1 [> 70:300 then {
	T -Ggreen
	0 0 1 s -W1,cyan -G-
}
if \$1 >= 300 then T -Gblue
# Then draw a circle width diameter based on magnitude ranges
if \$2 [> 0:1  then 0 0 0.1 c
if \$2 [> 1:2  then 0 0 0.2 c
if \$2 [> 2:3  then 0 0 0.3 c
if \$2 [> 3:4  then 0 0 0.4 c
if \$2 [> 4:5  then 0 0 0.5 c
if \$2 [> 5:6  then 0 0 0.6 c
if \$2 [> 6:7  then 0 0 0.7 c
if \$2 [> 7:8  then 0 0 0.8 c
if \$2 [> 8:9  then 0 0 0.9 c
if \$2 [> 9:10 then {
	0 0 1.0 c
	if \$1 == 50 then {
		0 0 0.2 t -Gwhite -Wthinnest
	}
} else {
	0 0 0.1 t -Gblack
}
EOF

# Plot a few quakes.

cat << EOF > q.txt
0	0	35	3
1	1	114	2
2	2	410	6
2	0	50	9.5
EOF
psxy -R-1/3/-1/3 -JM6i -P -Skseis/0.5i -Wthinnest q.txt -B1 > $ps

pscmp
