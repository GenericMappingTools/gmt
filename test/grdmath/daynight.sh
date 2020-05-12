#!/usr/bin/env bash
ps=daynight.ps
gmt grdmath -Rd -I1 30 20 0 DAYMASK = a.grd
gmt grdmath -Rd -I1 30 20 5 DAYMASK = b.grd
echo "0 black 1 white" > t.cpt
gmt grdimage a.grd -Ct.cpt -Baf -P -K -JQ6i > $ps
echo 30 20 | gmt psxy -R -J -O -K -Sc0.25c -Gred >> $ps
gmt grdimage b.grd -Ct.cpt -Baf -O -K -Y4i >> $ps
echo 30 20 | gmt psxy -R -J -O -K -Sc0.25c -Gred >> $ps
