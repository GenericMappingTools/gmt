#!/bin/bash
#
#	$Id: clipannot.sh,v 1.1 2011-03-21 21:14:53 guru Exp $

. ../functions.sh
header "Test grdcontour for clip path set by annotations"

ps=clipannot.ps
# Make a grid, annotate every 20 and use those as clip paths
# Then draw some lines intersection the annotations and draw
# a dense gridline set.  Text should not have lines through them.
grdmath -R0/10/0/10 -I0.1 X Y MUL = tmp.nc
grdcontour -A20+e+c0.05i+s24 -C10 tmp.nc -JX6i -P -K > $ps
psxy -R -J -O -K -W1p,red << EOF >> $ps
> line through 20 label
0	0
8.7	10
> line through 40 label
0	8.2
10	8.2
EOF
psbasemap -R -J -O -K -B0g0.2 >> $ps
psclip -Ct -O -K  >> $ps
psbasemap -R -J -O -B2WSne:."Delayed Annotations": >> $ps
rm -f tmp.nc

pscmp
