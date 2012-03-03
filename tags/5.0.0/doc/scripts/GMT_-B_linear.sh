#!/bin/bash
#	$Id$
#
. ./functions.sh

psbasemap -R0/12/0/0.95 -JX3i/0.3i -Ba4f2g1:Frequency::,%:S -K -P > GMT_-B_linear.ps
psxy -R -J -O -K -Sv2p+e+a60 -W0.5p -Gblack -Y0.1i -N << EOF >> GMT_-B_linear.ps
2 0 0 0.5
2 0 180 0.5
7 0 0 0.25
7 0 180 0.25
9.5 0 0 0.125
9.5 0 180 0.125
EOF
pstext -R -J -O -Gwhite -C0.01i/0.01i -F+f9p+jCB << EOF >> GMT_-B_linear.ps
2 0.2 annotation
7 0.2 frame
9.5 0.2 grid
EOF
