#!/bin/bash
#	$Id: GMT_-B_linear.sh,v 1.9 2011-05-17 00:23:50 guru Exp $
#
. ./functions.sh

psbasemap -R0/12/0/0.95 -JX3i/0.3i -Ba4f2g1:Frequency::,%:S -K -P > GMT_-B_linear.ps
psxy -R -J -O -K -Sv0.005i/0.02i/0.015i -Gblack -Y0.1i -N << EOF >> GMT_-B_linear.ps
2 0 0 0.5
2 0 180 0.5
7 0 0 0.25
7 0 180 0.25
9.5 0 0 0.125
9.5 0 180 0.125
EOF
pstext -R -J -O -Wwhite -C0.01i/0.01i -F+f9p+jCB << EOF >> GMT_-B_linear.ps
2 0.1 annotation
7 0.1 frame
9.5 0.1 grid
EOF
