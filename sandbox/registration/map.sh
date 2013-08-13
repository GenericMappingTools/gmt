#!/bin/sh
lon=90
ps=gmt_usage.ps
OP="1 DIV SIND NEG 0.1 MUL 0.15 SUB"
grdmath -R190/330/-90/90 -I30m -r X $lon SUB $OP = i.nc
grdcut -R190/330/-90/90 t30.nc -Gtmp.nc
grdgradient tmp.nc -Nt0.3 -A90 -Gint.nc
grdmath int.nc 0.75 ADD = int.nc
echo "-100000	steelblue	100000	steelblue" > t.cpt
#echo "-100000	lightgray	100000	lightgray" > t.cpt
grdimage i.nc -Iint.nc -Ct.cpt -Bg15 -Ji0.014i -K -P > $ps
psxy -R -J -O -K -W0.25p,75 ridge.d >> $ps
pscoast -R190/330/-90/90 -J -A10000 -Dc -Gdarkred -Wfaint -O -K >> $ps
grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -G255/220/0 >> $ps
#echo -157:59 21:55 | psxy -R -J -O -K -Sa0.1i -Gred -Wfaint >> $ps
#psxy -R -J -O -K t.txt -Sa0.1i -Gwhite -Wfaint >> $ps
grdmath -R-30/60/-90/90 -I30m -r X $lon SUB $OP = i.nc
grdcut -R-30/60/-90/90 t30.nc -Gtmp.nc
grdgradient tmp.nc -Nt0.3 -A90 -Gint.nc
grdmath int.nc 0.75 ADD = int.nc
grdimage i.nc -Iint.nc -Ct.cpt -B -Ji0.014i -X1.96i -O -K >> $ps
psxy -R -J -O -K -W0.25p,75 ridge.d >> $ps
pscoast -R-30/60/-90/90 -J -Dc -A10000 -Gdarkgreen -Wfaint -O -K >> $ps
grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -G255/220/0 >> $ps
#psxy -R -J -O -K t.txt -Sa0.1i -Gwhite -Wfaint >> $ps
grdmath -R60/190/-90/90 -I30m -r X $lon SUB $OP = i.nc
grdcut -R60/190/-90/90 t30.nc -Gtmp.nc
grdgradient tmp.nc -Nt0.3 -A90 -Gint.nc
grdmath int.nc 0.75 ADD = int.nc
grdimage i.nc -Iint.nc -Ct.cpt -B -Ji0.014i -X1.26i -O -K >> $ps
psxy -R -J -O -K -W0.25p,75 ridge.d >> $ps
pscoast -R60/190/-90/90 -J -Dc -A10000 -Gdarkblue -Wfaint -O -K >> $ps
grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -G255/220/0 >> $ps
#psxy -R -J -O -K t.txt -Sa0.1i -Gwhite -Wfaint >> $ps
#date +%x | awk '{print 0.05, 0.05, $1}' | pstext -R0/5/0/5 -Jx1i -F+f10p,Helvetica+jLB -O -Gcornsilk -TO -W0.25p -X-3.08i >> $ps
date +"@#GMT5 cumulative downloads from initial release to %d %B %Y@#" | awk '{print 10, -85, $0}' | pstext -R-30/60/-90/90 -J -F+f10p,Helvetica+jCB -O -K -Gcornsilk -N -TO -W0.25p -X-1.26i >> $ps
psxy -R -J -O -T >> $ps
ps2raster -E150 -A -TG $ps
rm -f $ps
open gmt_usage.png
