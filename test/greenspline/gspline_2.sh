#!/bin/bash
#
#       $Id: gspline_2.sh,v 1.1 2011-06-17 03:23:30 guru Exp $

. ../functions.sh
header "greenspline: Testing Cartesian 2-D interpolation I"

ps=gspline_2.ps

# Figure 3 in Wessel, P. (2009), A general-purpose Green's function-based
#	interpolator, Computers & Geosciences, 35, 1247–1254.

D=0.05
R=0/7.2/-0.2/7
T=../../doc/examples/ex12/table_5.11
greenspline -R$R -I$D -GWB1998.grd $T -St0.5 -D1
greenspline -R$R -I$D -GMM1993.grd $T -Sr0.99 -D1
surface $T -R$R -I$D -Graws5.grd -T0.5 -N10000 -C0.0000001
grdcontour -R raws5.grd -Jx0.9i -P -K -C25 -A50+jCB -Ba2f1WSne -Y2i -GlLB/CT  > $ps
grdcontour -R WB1998.grd -J -O -K -C25 -A50+jCB -Wa0.75p,- -Wc0.25p,-  -GlCT/RB >> $ps
grdcontour -R MM1993.grd -J -O -K -C25 -Wa0.75p,. -Wc0.25p,. >> $ps
psxy -R -J -O $T -Sc0.1 -G0 >> $ps
rm -f WB1998.grd raws5.grd MM1993.grd

pscmp
