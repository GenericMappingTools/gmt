#!/usr/bin/env bash
# A contour interpolated in a projected triangle can span a periodic map.
cat << EOF > triangle.xyz
180 -30 -30
180 30 30
-180 -30 -30
EOF
gmt pscontour triangle.xyz -R-180/180/-30/30 -JQ0/10c -fg -C-15, -A- -W2p,magenta -P -Baf -BWSen > triangle.ps
gmt psconvert triangle.ps -A -Tg -E144

# The long part of the contour runs through the center column of the image.
height=$("$GRAPHICSMAGICK" identify -ping -format '%h' triangle.png)
if ! (set -o pipefail; "$GRAPHICSMAGICK" convert triangle.png -gravity Center -crop "1x${height}+0+0" -depth 8 txt:- | "$AWK" 'toupper($NF) == "#FF00FF" { found = 1 } END { exit !found }'); then
	echo 'The projected contour did not reach the center column' >&2
	return 1
fi
