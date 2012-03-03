#!/bin/sh
#	$Id$
# Testing trend2d

. ../functions.sh
header "Test trend2 by removing a robust quadratic plane"

ps=trend.ps
makecpt -Crainbow -T690/960/10 > z.cpt
makecpt -Cjet -T-60/120/10 > r.cpt
makecpt -Chot -T0.7/1/0.02 > w.cpt
pscontour -R0/6.5/0/6.5 ${GMT_SOURCE_DIR}/doc/examples/ex16/table_5.11 -C25 -A50 -JX3i -Y6.5i -Baf:.Data: -K -X1i -I -Cz.cpt -P > $ps
triangulate -M ${GMT_SOURCE_DIR}/doc/examples/ex16/table_5.11 | psxy -R -J -O -K -W0.25p,- >> $ps
psxy -R -J -O -K ${GMT_SOURCE_DIR}/doc/examples/ex16/table_5.11 -Sc0.1c -Gblack >> $ps
psscale -Cz.cpt -D1.5i/-0.5i/3i/0.1ih -O -K -Ba >> $ps
trend2d ${GMT_SOURCE_DIR}/doc/examples/ex16/table_5.11 -Fxyrmw -N3r > trend.txt
pscontour -R trend.txt -Cr.cpt -J -Baf:.Redisual: -I -O -K -X3.5i -i0-2 >> $ps
psscale -Cr.cpt -D1.5i/-0.5i/3i/0.1ih -O -K -Ba >> $ps
pscontour -R trend.txt -Cz.cpt -J -Baf:.Trend: -I -O -K -X-3.5i -Y-5i -i0,1,3 >> $ps
psscale -Cz.cpt -D1.5i/-0.5i/3i/0.1ih -O -K -Ba >> $ps
pscontour -R trend.txt -Cw.cpt -J -Baf:.Weights: -I -O -K -X3.5i -i0,1,4 >> $ps
psscale -Cw.cpt -D1.5i/-0.5i/3i/0.1ih -O -K -Ba >> $ps
psxy -R$Rp -J -O -T >> $ps
pscmp
rm -f trend.txt ?.cpt
