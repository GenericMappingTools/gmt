#!/usr/bin/env bash

ps=gspline_6.ps

# Test for the bilinear spline option -Sl.

D=0.05
R=0/7.2/-0.2/7
gmt greenspline -R$R -I$D -GWB1998.grd @Table_5_11.txt -Sl -Z1
gmt surface @Table_5_11.txt -R$R -I$D -Graws5.grd -T0.5 -N10000 -C0.0000001
gmt grdcontour -R raws5.grd -Jx0.9i -P -K -C25 -A50+jCB+n0/0.1c -Ba2f1 -BWSne -Y2i -GlLB/CT  > $ps
gmt grdcontour -R WB1998.grd -J -O -K -C25 -A50+jCB+n0/0.1c -Wa0.75p,- -Wc0.25p,-  -GlCT/RB >> $ps
gmt psxy -R -J -O @Table_5_11.txt -Sc0.1 -G0 >> $ps
