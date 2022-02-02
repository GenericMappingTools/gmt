#!/usr/bin/env bash
# Test script demonstrating problem discussed in message #7065 [Sabin].
# grdmask does not create the mask for the polygon in the W hemisphere (?)
# DVC_TEST
ps=twoblobs.ps
gmt makecpt -T0,1,2 -Cwhite,red > t.cpt
cat << EOF > two.txt
> I straddle Greenwhich
2	40
10	40
10	35
1	35
-5	35
-5	40
2	40
> I am in the south Pacific
-135	-20
-125	-20
-125	-30
-135	-30
-135	-20
EOF
gmt psxy -R-180/180/-60/60 -JM6i -P -Bafg -B+t"Plot polygons with psxy" -K two.txt -Gred > $ps

gmt grdmask -R two.txt -Gout.nc -N0/1/1 -I20k
gmt grdimage -R -J -Ct.cpt out.nc -Bafg -B+t"Plot polygons via grdmask/image" -O -Y5i >> $ps
