#!/usr/bin/env bash
#
# Check custom symbol needed 2 column strings from trailing text
# and using size to scale the texts.  Note words must be separated
# by a single tab or space

cat << EOF > t.txt
0	0
60	60
EOF

gmt begin angled_text
	gmt set MAP_FRAME_TYPE plain
	gmt plot t.txt -R-5/65/-5/65 -JX12c -B -Sl14p+tHELLO+a30 -Gblack
	gmt plot t.txt -JQ12c -B -Sl14p+tHELLO+A30 -Y13c -Gblack
gmt end show

