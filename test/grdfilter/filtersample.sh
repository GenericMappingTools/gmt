#!/bin/bash
ps=filtersample.ps

echo "0.5 0.5 1" | gmt xyz2grd -N0 -I0.1 -An -Gtemp.grd -R0/1/0/1

gmt grdfilter temp.grd -Gtemp2.grd -R0/1/0/1 -I0.01 -D0 -Fg0.5
gmt grd2cpt temp2.grd -E100 -D -Cjet > sn.cpt
gmt grdimage temp2.grd -Baf -BWSne -P -JX4i -K -Csn.cpt -Xc  > $ps
gmt grdimage temp.grd  -Baf -BWSne -J -O -Csn.cpt -Y5i >> $ps
