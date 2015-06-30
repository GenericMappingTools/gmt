#!/bin/bash
#		GMT EXAMPLE 16
#		$Id$
#
# Purpose:	Illustrates interpolation methods using same data as Example 12.
# GMT progs:	gmtset, grdview, grdfilter, pscontour, psscale, pstext, surface, triangulate
# Unix progs:	echo, rm
#
# Illustrate various means of contouring, using triangulate and surface.
#
ps=example_16.ps
gmt gmtset FONT_ANNOT_PRIMARY 9p
#
gmt pscontour -R0/6.5/-0.2/6.5 -Jx0.45i -P -K -Y5.5i -Ba2f1 -BWSne table_5.11 -Cex16.cpt -I > $ps
echo "3.25 7 pscontour (triangulate)" | gmt pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> $ps
#
gmt surface table_5.11 -R -I0.2 -Graws0.nc
gmt grdview raws0.nc -R -J -B -Cex16.cpt -Qs -O -K -X3.5i >> $ps
echo "3.25 7 surface (tension = 0)" | gmt pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> $ps
#
gmt surface table_5.11 -R -I0.2 -Graws5.nc -T0.5
gmt grdview raws5.nc -R -J -B -Cex16.cpt -Qs -O -K -Y-3.75i -X-3.5i >> $ps
echo "3.25 7 surface (tension = 0.5)" | gmt pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> $ps
#
gmt triangulate table_5.11 -Grawt.nc -R -I0.2 > /dev/null
gmt grdfilter rawt.nc -Gfiltered.nc -D0 -Fc1
gmt grdview filtered.nc -R -J -B -Cex16.cpt -Qs -O -K -X3.5i >> $ps
echo "3.25 7 triangulate @~\256@~ grdfilter" | gmt pstext -R -J -O -K -N \
	-F+f18p,Times-Roman+jCB >> $ps
echo "3.2125 7.5 Gridding of Data" | gmt pstext -R0/10/0/10 -Jx1i -O -K -N \
	-F+f32p,Times-Roman+jCB -X-3.5i >> $ps
gmt psscale -Dx3.25i/0.35i+jTC+w5i/0.25i+h -Cex16.cpt -O -Y-0.75i >> $ps
#
rm -f *.nc gmt.conf
