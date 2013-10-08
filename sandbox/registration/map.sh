#!/bin/sh
ps=gmt_usage.ps
if [ ! -f background_map.ps ]; then
	# This is how the background map is built once, we then use the PS file in the script
	lon=90
	OP="1 DIV SIND NEG 0.1 MUL 0.15 SUB"
	#grdfilter -V -I30m etopo2m_grd.nc -Fg100 -Gt30.nc -D4 -T
	grdmath -R190/330/-90/90 -I30m -r X $lon SUB $OP = i.nc
	grdcut -R190/330/-90/90 t30.nc -Gtmp.nc
	grdgradient tmp.nc -Nt0.3 -A90 -Gint.nc
	grdmath int.nc 0.75 ADD = int.nc
	echo "-100000	steelblue	100000	steelblue" > t.cpt
	#echo "-100000	lightgray	100000	lightgray" > t.cpt
	grdimage i.nc -Iint.nc -Ct.cpt -Bg15 -Ji0.014i -K -P > background_map.ps
	grdmath -R-30/60/-90/90 -I30m -r X $lon SUB $OP = i.nc
	grdcut -R-30/60/-90/90 t30.nc -Gtmp.nc
	grdgradient tmp.nc -Nt0.3 -A90 -Gint.nc
	grdmath int.nc 0.75 ADD = int.nc
	grdimage i.nc -Iint.nc -Ct.cpt -B -Ji0.014i -X1.96i -O -K >> background_map.ps
	grdimage i.nc -Iint.nc -Ct.cpt -Bg15 -Ji0.014i -K -P > $ps
	grdmath -R60/190/-90/90 -I30m -r X $lon SUB $OP = i.nc
	grdcut -R60/190/-90/90 t30.nc -Gtmp.nc
	grdgradient tmp.nc -Nt0.3 -A90 -Gint.nc
	grdmath int.nc 0.75 ADD = int.nc
	grdimage i.nc -Iint.nc -Ct.cpt -B -Ji0.014i -X1.26i -O -K >> background_map.ps
	psxy -R -J -O -K -T -X-3.22i >> background_map.ps
	#rm -f t30.nc i.nc tmp.nc int.nc t.cpt
fi
cp -f background_map.ps $ps
psxy -R-30/60/-90/90 -Ji0.014i -O -K -W0.25p,75 ridge.d >> $ps
pscoast -R190/330/-90/90 -J -A10000 -Dc -Gdarkred -Wfaint -O -K >> $ps
grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -G255/220/0 >> $ps
psxy -R -J -O -K -W0.25p,75 ridge.d -X1.96i >> $ps
pscoast -R-30/60/-90/90 -J -Dc -A10000 -Gdarkgreen -Wfaint -O -K >> $ps
grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -G255/220/0 >> $ps
psxy -R -J -O -K -W0.25p,75 ridge.d -X1.26i >> $ps
pscoast -R60/190/-90/90 -J -Dc -A10000 -Gdarkblue -Wfaint -O -K >> $ps
grep -v '^#' GMT_old_unique_sites.d | psxy -R -J -O -K -Sc0.02i -G255/220/0 >> $ps
date +"@#GMT5 cumulative downloads from initial release to %d %B %Y@#" | awk '{print 10, -85, $0}' | pstext -R-30/60/-90/90 -J -F+f10p,Helvetica+jCB -O -K -Gcornsilk -N -TO -W0.25p -X-1.26i >> $ps
psxy -R -J -O -T >> $ps
ps2raster -E150 -A -TG $ps
rm -f $ps
open gmt_usage.png
