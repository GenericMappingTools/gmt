#!/bin/bash
# Test grdlandmask for border tracing [gridline grid]
ps=trace1g.ps
R=9/11/36/38
cat << EOF > t.cpt
0	red
1	blue
2	lightbrown
3	cyan
4	white
EOF
gmt grdlandmask -R$R -Di -I1m -r -N1/2/3/4/5 -Gt.nc -E0/4/5/6
gmt grdimage t.nc -JM6i -nn -K -Ct.cpt -Xc -t75 -P > $ps
gmt pscoast -R -J -Di -Wthin -O -Bafg1m -BWSne --MAP_GRID_PEN=faint >> $ps
