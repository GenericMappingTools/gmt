#!/usr/bin/env bash
# Test that -Q and -Qr both report the grand total as the area

cat << EOF > cumulative_area.txt
4.7
8.1
12.6
15.3
18.9
21.2
24.8
27.4
29.1
31.7
33.5
35.2
37.8
39.4
41.1
42.6
44.3
45.9
47.2
48.8
50.1
51.6
53.2
54.7
56.3
57.9
59.4
61.2
63.5
65.1
67.8
70.3
72.6
75.1
78.4
81.7
85.2
88.6
92.3
96.8
EOF
n=$(wc -l < cumulative_area.txt)

get_area () {
	gmt pshistogram cumulative_area.txt -T0/100/10 -Z0 "$1" -JX6i/4i -W1p -N0 -Vi 2>&1 >/dev/null | \
		sed -n 's/.*Area under histogram is \(.*\)/\1/p'
}

fwd=$(get_area -Q)
rev=$(get_area -Qr)

echo "$n $fwd $rev" | awk '{
	if ($2 != $1) { print "FAIL: -Q area =", $2, "want", $1; err = 1 }
	else print "PASS: -Q area =", $2
	if ($3 != $1) { print "FAIL: -Qr area =", $3, "want", $1; err = 1 }
	else print "PASS: -Qr area =", $3
	exit err + 0
}'
