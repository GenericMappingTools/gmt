#!/usr/bin/env bash
# Test grdmask with search radius around some points
# Made because of http://gmt.soest.hawaii.edu/boards/1/topics/5379
# DVC_TEST
ps=geoholes.ps
# 20 degree radii for all, gridline-registered
gmt grdmask -Gmask.grd -I1 -R0/360/-90/90 -N1/1/NaN -S20d << EOF
0	40
355	-40
133	-5
-63	-10
EOF
gmt makecpt -Cjet -T0/10 > t.cpt
gmt grdimage mask.grd -JH0/6i -Ct.cpt -Baf -P -K -Xc -Y0.5i > $ps
# Variable degree radii for all, gridline-registered
gmt grdmask -Gmask.grd -I1 -R0/360/-90/90 -N1/1/NaN -Szd << EOF
0	40	20
355	-40	15
133	-5	30
-63	-10	10
EOF
gmt grdimage mask.grd -JH0/6i -Ct.cpt -Baf -O -K -Y3.25i >> $ps
# Variable km radii for all, pixel-registered
gmt grdmask -Gmask.grd -I1 -R0/360/-90/90 -N1/1/NaN -Szk -r << EOF
0	40	2000
355	-40	1500
133	-5	3000
-63	-10	1000
EOF
gmt grdimage mask.grd -JH0/6i -Ct.cpt -Baf -O -Y3.25i >> $ps
