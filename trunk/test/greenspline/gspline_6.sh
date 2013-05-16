#!/bin/bash
#
#       $Id$

ps=gspline_6.ps

# Test for the bilinear spline option -Sl.

D=0.05
R=0/7.2/-0.2/7
T=../../doc/examples/ex12/table_5.11
greenspline -R$R -I$D -GWB1998.grd $T -Sl -D1
surface $T -R$R -I$D -Graws5.grd -T0.5 -N10000 -C0.0000001
grdcontour -R raws5.grd -Jx0.9i -P -K -C25 -A50+jCB -Ba2f1 -BWSne -Y2i -GlLB/CT  > $ps
grdcontour -R WB1998.grd -J -O -K -C25 -A50+jCB -Wa0.75p,- -Wc0.25p,-  -GlCT/RB >> $ps
psxy -R -J -O $T -Sc0.1 -G0 >> $ps
