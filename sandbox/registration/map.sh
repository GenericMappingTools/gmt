#!/bin/sh
# Test script for developing background map.
# This will be pasted into gmt_map_geoip.sh when working OK

ps=gmt_usage.ps
map=wiki_background_map.ps
if [ ! -f wiki_background_map.ps ]; then
	# This is how the background map is built once, we then use the PS file in the script
	# For convenience we store this map in svn and only rebuild if we change our mind about
	# colors etc.  We use a scale of 0.028c/degree, resulting in a 10.08 cm wide map
	RED=darkred
	GREEN=darkgreen
	BLUE=darkblue
	LAKE=35000
	lon=90
	OP="1 DIV SIND NEG 0.1 MUL 0.15 SUB"
	#grdfilter -V -I30m etopo2m_grd.nc -Fg100 -Gt30.nc -D4 -T
	# LEFT image
	grdmath -R190/330/-90/90 -I30m -r X $lon SUB $OP = i.nc
	grdcut -R190/330/-90/90 t30.nc -Gtmp.nc
	grdgradient tmp.nc -Nt0.3 -A90 -Gint.nc
	grdmath int.nc 0.75 ADD = int.nc
	echo "-100000	steelblue	100000	steelblue" > t.cpt
	grdimage i.nc -Iint.nc -Ct.cpt -Bg15 -Ji0.028c -K -P > $map
	pscoast -R190/330/-90/90 -J -A$LAKE -Dc -G$RED -Wfaint -O -K >> $map
	psxy -R -J -O -K -W0.25p,75 ridge.d >> $map
	# MIDDLE image
	grdmath -R-30/60/-90/90 -I30m -r X $lon SUB $OP = i.nc
	grdcut -R-30/60/-90/90 t30.nc -Gtmp.nc
	grdgradient tmp.nc -Nt0.3 -A90 -Gint.nc
	grdmath int.nc 0.75 ADD = int.nc
	grdimage i.nc -Iint.nc -Ct.cpt -Bg15 -J -X3.92c -O -K >> $map
	pscoast -R -J -Dc -A$LAKE -G$GREEN -Wfaint -O -K >> $map
	psxy -R -J -O -K -W0.25p,75 ridge.d >> $map
	# RIGHT image
	grdmath -R60/190/-90/90 -I30m -r X $lon SUB $OP = i.nc
	grdcut -R60/190/-90/90 t30.nc -Gtmp.nc
	grdgradient tmp.nc -Nt0.3 -A90 -Gint.nc
	grdmath int.nc 0.75 ADD = int.nc
	grdimage i.nc -Iint.nc -Ct.cpt -B -J -X2.52c -O -K >> $map
	pscoast -R -J -Dc -A$LAKE -G$BLUE -Wfaint -O -K >> $map
	psxy -R -J -O -K -W0.25p,75 ridge.d >> $map
	psxy -R -J -O -K -T -X-6.44c >> $map
	#rm -f t30.nc i.nc tmp.nc int.nc t.cpt
fi
cp -f $map $ps
if [ $1 -eq 1 ]; then
	grep -v '^#' GMT_old_unique_sites.d | psxy -R190/330/-90/90 -Ji0.028c -O -K -Sc0.05c -G255/220/0 >> $ps
	grep -v '^#' GMT_old_unique_sites.d | psxy -R-30/60/-90/90 -J -O -K -Sc0.05c -G255/220/0 -X3.92c >> $ps
	grep -v '^#' GMT_old_unique_sites.d | psxy -R60/190/-90/90 -J -O -K -Sc0.05c -G255/220/0 -X2.52c >> $ps
	#date +"@#GMT5 cumulative downloads from initial release to %d %B %Y@#" | awk '{print 10, -85, $0}' | 
	# pstext -R-30/60/-90/90 -J -F+f10p,Helvetica+jCB -O -K -Gcornsilk -N -TO -W0.25p -X-2.52c >> $ps
fi
psxy -R -J -O -T >> $ps
ps2raster -E150 -A -TG $ps
#rm -f $ps
open gmt_usage.png
