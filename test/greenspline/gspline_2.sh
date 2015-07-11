#!/bin/bash
#
#       $Id$

ps=gspline_2.ps

# Figure 3 in Wessel, P. (2009), A general-purpose Green's function-based
#	interpolator, Computers & Geosciences, 35, 1247-1254.

D=0.05
R=0/7.2/-0.2/7
T=${GMT_SOURCE_DIR}/doc/examples/ex12/table_5.11
gmt greenspline -R$R -I$D -GWB1998.grd $T -St0.5 -D1
gmt greenspline -R$R -I$D -GMM1993.grd $T -Sr0.99 -D1
gmt surface $T -R$R -I$D -Graws5.grd -T0.5 -N10000 -C0.0000001
gmt grdcontour -R raws5.grd -Jx0.9i -P -K -C25 -A50+jCB+n0/0.1c -Ba2f1 -BWSne -Y2i -GlLB/CT  > $ps
gmt grdcontour -R WB1998.grd -J -O -K -C25 -A50+jCB+n0/0.1c -Wa0.75p,- -Wc0.25p,-  -GlCT/RB >> $ps
gmt grdcontour -R MM1993.grd -J -O -K -C25 -Wa0.75p,. -Wc0.25p,. >> $ps
gmt psxy -R -J -O $T -Sc0.1 -G0 >> $ps

