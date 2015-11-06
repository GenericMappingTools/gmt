#!/bin/bash
#	$Id$
# Testing tiling of 2 small JPG images (64x64 and 90x90 pixels)

ps=tiling.ps
dpi=300

cat << EOF > tiling.txt
> -Gp$dpi/${src:-.}/tiling1.jpg
-2	-2
2	-2
0	0
> -Gp$dpi/${src:-.}/tiling2.jpg
0	0
2	-2
2	2
EOF

gmt psxy -R-3/3/-3/3 -Jx1i -Baf -P -L -Wthin tiling.txt -Xc > $ps
