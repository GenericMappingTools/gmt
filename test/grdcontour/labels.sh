#!/usr/bin/env bash
#
#
# Test dumping of contour labels and replotting in pstext
# DVC_TEST

ps=labels.ps

gmt grdmath -R0/10/0/10 -I1 X Y MUL = t.nc
gmt grdcontour t.nc -JX4i -P -Baf -A10+tlabels.txt -GlBL/TR -K -Xc > $ps
gmt pstext -R -J -O -Baf -F+A+f12p labels.txt -Y4.75i >> $ps
