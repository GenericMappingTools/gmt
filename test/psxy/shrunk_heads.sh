#!/usr/bin/env bash
# Exploring the terminal limit on vector shrinking for geo and Cartesian vectors
#
gmt begin shrunk_heads
	gmt set PROJ_LENGTH_UNIT inch MAP_FRAME_TYPE plain
	gmt math -N4 -T0/0.5/0.025 -C3 T 40 MUL = t.txt
	gmt plot t.txt -S=32p+e+h0.5+a20+n15/0.5 -Gblack -W2p -R-0.04/0.55/-0.02/0.20 -JM6i -Bafg10
	echo GEO | gmt text -F+f14p+cTL -Dj0.2c -Gwhite
	gmt plot t.txt -S=32p+eA+h0.5+a20+n15/0.5 -W2p -Bafg10 -Y2.4i -BWsrt
	echo GEO | gmt text -F+f14p+cTL -Dj0.2c -Gwhite
	gmt math -N4 -T0/0.5/0.025  -C3 T 4 MUL -C2 0 MUL 90 ADD = t.txt
	gmt plot t.txt -Sv32p+e+h0.5+a20+n1.5i/0.5 -Gblack -W2p -R-0.04/0.55/-0.02/0.2 -JX6i/0 -Bafg10 -Y2.4i -BWsrt
	echo CART | gmt text -F+f14p+cTL -Dj0.2c -Gwhite
	gmt plot t.txt -Sv32p+eA+h0.5+a20+n1.5i/0.5 -W2p -Bafg10 -Y2.35i -BWsrN
	echo CART | gmt text -F+f14p+cTL -Dj0.2c -Gwhite
gmt end show
