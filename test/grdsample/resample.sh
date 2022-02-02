#!/usr/bin/env bash
# Test to ensure https://forum.generic-mapping-tools.org/t/grdsample-problem-in-gmt-6-0/552/14 is fixed
# DVC_TEST
ps=resample.ps
gmt grdmath -R0/360/-90/90 -I0.25 X = t.grd
gmt grdsample -R-50/50/25/50 -fg t.grd -Gsample1.grd
gmt grdsample -R50/150/25/50 -fg t.grd -Gsample2.grd
gmt grdsample -R310/410/25/50 -fg t.grd -Gsample3.grd
gmt makecpt -Chot -T0/360 > t.cpt
gmt grdimage t.grd -JQ180/15c -P -Baf -BWSne -Ct.cpt -K -Y2c -X3c > $ps
gmt grdimage sample2.grd -J -Baf -BWSne -Ct.cpt -O -K -Y9.3c >> $ps
gmt grdimage sample1.grd -R-50/50/25/50 -J -Baf -BWSne -Ct.cpt -O -K -Y5.3c >> $ps
gmt grdimage sample3.grd -R310/410/25/50 -J -Baf -BWSne -Ct.cpt -O -Y5.3c >> $ps
