#!/bin/sh
#	$Id: GMT_-B_linear.sh,v 1.3 2004-04-10 17:19:14 pwessel Exp $
#

psbasemap -R0/12/0/0.95 -JX3/0.3 -Ba4f2g1:Frequency::,%:S -K -P > GMT_-B_linear.ps
psxy -R -JX -O -K -Sv0.005/0.02/0.015 -Gblack -Y0.1i -N -V << EOF >> GMT_-B_linear.ps
2 0 0 0.5
2 0 180 0.5
7 0 0 0.25
7 0 180 0.25
9.5 0 0 0.125
9.5 0 180 0.125
EOF
pstext -R -JX -O -W255 -C0.01/0.01 << EOF >> GMT_-B_linear.ps
2 0.1 9 0 0 CB annotation
7 0.1 9 0 0 CB frame
9.5 0.1 9 0 0 CB grid
EOF
