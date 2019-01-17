#!/usr/bin/env bash
#

ps=clipannot.ps

# Make a grid, annotate every 20 and use those as clip paths
# Then draw some lines intersection the annotations and draw
# a dense gridline set.  Text should not have lines through them.
gmt grdmath -R0/10/0/10 -I0.1 X Y MUL = tmp.nc
gmt grd2xyz tmp.nc > tmp.xyz
gmt pscontour -R0/10/0/10 -A20+e+c0.05i+f24p -C10 tmp.xyz -JX6i -P -K -W0.5p > $ps
gmt psxy -R -J -O -K -W1p,red << EOF >> $ps
> line through 20 label
0	0
10	8.5
> line through 40 label
0	8.2
10	8.2
EOF
gmt psbasemap -R -J -O -K -Bg0.2 >> $ps
gmt psclip -C -O -K  >> $ps
gmt psbasemap -R -J -O -B2 -BWSne+t"Delayed Annotations" >> $ps

