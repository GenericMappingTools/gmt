#!/usr/bin/env bash
# Test gmtselect with points inside or outside a grid
ps=inoutgrid.ps
gmt xyz2grd -R0/5/0/5 -I1 -r -Gmask.nc << EOF
0.5	0.5	1
1.5	0.5	1
2.5	0.5	1
3.5	0.5	1
0.5	1.5	1
1.5	1.5	1
3.5	1.5	1
0.5	2.5	1
1.5	2.5	1
2.5	2.5	1
3.5	2.5	1
3.5	3.5	1
EOF
# Created the test data via RAND then saved
#gmt math -T0/25/1 -C0 0 5 RAND -C1 0 5 RAND ADD = locations.txt
echo 0	pink	1 pink > t.cpt
# Get what gmtselect lets through
gmt select @locations.txt -Gmask.nc > in.txt
gmt select @locations.txt -Gmask.nc -Ig > out.txt
gmt grdimage mask.nc -JX6i -P -Bafg1 -Ct.cpt -Q -K > $ps
# Lay down inside points
gmt psxy -Rmask.nc -J -O -K in.txt  -Sc0.1i -Gwhite -W0.25p >> $ps
gmt psxy -Rmask.nc -J -O -K out.txt -Sc0.1i -Gblack -W0.25p >> $ps
