#!/usr/bin/env bash
# Test grdmath stacking mode -S for available operators
# DVC_TEST
ps=reduce.ps
gmt set PS_MEDIA letter MAP_TITLE_OFFSET 4p FONT_TITLE 12p
# Create 3 small grids with integers in 0-100 range
cat << EOF | gmt xyz2grd -R0/2/0/2 -I1 -r -Z -G1.grd
43
31
82
82
EOF
cat << EOF | gmt xyz2grd -R0/2/0/2 -I1 -r -Z -G2.grd
96
74
78
79
EOF
cat << EOF | gmt xyz2grd -R0/2/0/2 -I1 -r -Z -G3.grd
77
71
2
48
EOF
# Plot the three grids on top
gmt grd2xyz 1.grd | gmt pstext -R0/2/0/2 -JX1.75i -P -B0g1 -B+t1 -K -Y8.5i -F+f18p+jCM > $ps
gmt grd2xyz 2.grd | gmt pstext -R -J -O -K -B0g1 -B+t2 -X2.25i -F+f18p+jCM >> $ps
gmt grd2xyz 3.grd | gmt pstext -R -J -O -K -B0g1 -B+t3 -X2.25i -F+f18p+jCM >> $ps
# Plot MEAN MEDIAN MODE on next row
gmt grdmath [123].grd -S MEAN = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -B0g1 -B+tMEAN -K -Y-2i -X-4.5i -F+f18p+jCM+z%0.1f >> $ps
gmt grdmath [123].grd -S MEDIAN = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -K -B0g1 -B+tMEDIAN -X2.25i -F+f18p+jCM+z%0.1f >> $ps
gmt grdmath [123].grd -S MODE = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -K -B0g1 -B+tMODE -X2.25i -F+f18p+jCM+z%0.1f >> $ps
# Plot STD, MAD, LMSSCL on next row
gmt grdmath [123].grd -S STD = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -B0g1 -B+tSTD -K -Y-2i -X-4.5i -F+f18p+jCM+z%0.1f >> $ps
gmt grdmath [123].grd -S MAD = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -K -B0g1 -B+tMAD -X2.25i -F+f18p+jCM+z%0.1f >> $ps
gmt grdmath [123].grd -S LMSSCL = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -K -B0g1 -B+tLMSSCL -X2.25i -F+f18p+jCM+z%0.1f >> $ps
# Plot MIN, MAX, RMS on next row
gmt grdmath [123].grd -S MIN = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -B0g1 -B+tMIN -K -Y-2i -X-4.5i -F+f18p+jCM+z%0.1f >> $ps
gmt grdmath [123].grd -S MAX = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -K -B0g1 -B+tMAX -X2.25i -F+f18p+jCM+z%0.1f >> $ps
gmt grdmath [123].grd -S RMS = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -K -B0g1 -B+tRMS -X2.25i -F+f18p+jCM+z%0.1f >> $ps
# Plot ADD, SUB, AND on next row
gmt grdmath [123].grd -S ADD = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -B0g1 -B+tADD -K -Y-2i -X-4.5i -F+f18p+jCM+z%0.1f >> $ps
gmt grdmath [123].grd -S SUB = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -K -B0g1 -B+tSUB -X2.25i -F+f18p+jCM+z%0.1f >> $ps
gmt grdmath [123].grd -S AND = tmp.grd
gmt grd2xyz tmp.grd | gmt pstext -R -J -O -B0g1 -B+tAND -X2.25i -F+f18p+jCM+z%0.1f >> $ps
