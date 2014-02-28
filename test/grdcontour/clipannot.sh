#!/bin/bash
#
#	$Id$

ps=clipannot.ps

# Make a grid, annotate every 20 and use those as clip paths
# Then draw some lines intersection the annotations and draw
# a dense gridline set.  Text should not have lines through them.
gmt grdmath -R0/10/0/10 -I0.1 X Y MUL = tmp.nc
gmt grdcontour -A20+e+c0.05i+f24p -C10 tmp.nc -JX6i -P -K > $ps
gmt psxy -R -J -O -K -W1p,red << EOF >> $ps
> line through 20 label
0	0
8.7	10
> line through 40 label
0	8.2
10	8.2
EOF
gmt psbasemap -R -J -O -K -Bg0.2 >> $ps
gmt psclip -Cs -O -K  >> $ps
gmt psbasemap -R -J -O -B2 -BWSne+t"Delayed Annotations" >> $ps

