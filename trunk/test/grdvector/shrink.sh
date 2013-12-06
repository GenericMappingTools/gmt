#!/bin/bash
# $Id$
# Test shrinking of line and heads with/without +n for grdvector
ps=shrink.ps
gmt grdmath -R0/360/-30/60 -I30/30 -r 7 X MUL = x.nc
gmt grdmath -R0/360/-30/60 -I30/30 -r 100 = y.nc
cat << EOF > line
180	-30
180	60
EOF
gmt grdvector x.nc y.nc -JM6i -Si1 -Q0.1i+n1260 -W1.5p -Gred -Ba -BWSne -P -K -Xc > $ps
gmt psxy -R -J -O -K -W0.5p,blue line >> $ps
gmt grdvector x.nc y.nc -R -J -Si1 -Q0.1i+b+e+n1260+jc -W0.5p -Gred -Ba -BWSne -O -K -Y2.3i >> $ps
gmt psxy -R -J -O -K -W0.5p,blue line >> $ps
gmt grdvector x.nc y.nc -R -J -Si1 -Q0.1i+e -W1p -Gred -Ba -BWSne -O -K -Y2.3i >> $ps
#psxy -R -J -O -K -W0.5p,blue line >> $ps
gmt grdvector x.nc y.nc -R -J -Si1 -Q0.1i+e+n1260 -W1p -Gred -Ba -BWSne+t"Shrink <----> Constant" -O -K -Y2.3i >> $ps
gmt psxy -R -J -O -W0.5p,blue line >> $ps
