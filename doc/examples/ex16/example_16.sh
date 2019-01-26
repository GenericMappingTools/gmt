#!/usr/bin/env bash
#		GMT EXAMPLE 16
#
# Purpose:	Illustrates interpolation methods using same data as Example 12.
# GMT modules:	gmtset, grdview, grdfilter, pscontour, psscale, pstext, surface, triangulate
# Unix progs:	echo, rm
#
# Illustrate various means of contouring, using triangulate and surface.
#
ps=example_16.ps
gmt set FONT_ANNOT_PRIMARY 9p
#
gmt pscontour -R0/6.5/-0.2/6.5 -Jx0.45i -P -K -Y5.5i -Ba2f1 -BWSne @Table_5_11.txt -C@ex_16.cpt -I > $ps
echo "3.25 7 pscontour (triangulate)" | gmt pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> $ps
#
gmt surface @Table_5_11.txt -R -I0.2 -Graws0.nc
gmt grdview raws0.nc -R -J -B -C@ex_16.cpt -Qs -O -K -X3.5i >> $ps
echo "3.25 7 surface (tension = 0)" | gmt pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> $ps
#
gmt surface @Table_5_11.txt -R -I0.2 -Graws5.nc -T0.5
gmt grdview raws5.nc -R -J -B -C@ex_16.cpt -Qs -O -K -Y-3.75i -X-3.5i >> $ps
echo "3.25 7 surface (tension = 0.5)" | gmt pstext -R -J -O -K -N -F+f18p,Times-Roman+jCB >> $ps
#
gmt triangulate @Table_5_11.txt -Grawt.nc -R -I0.2
gmt grdfilter rawt.nc -Gfiltered.nc -D0 -Fc1
gmt grdview filtered.nc -R -J -B -C@ex_16.cpt -Qs -O -K -X3.5i >> $ps
echo "3.25 7 triangulate @~\256@~ grdfilter" | gmt pstext -R -J -O -K -N \
	-F+f18p,Times-Roman+jCB >> $ps
echo "3.2125 7.5 Gridding of Data" | gmt pstext -R0/10/0/10 -Jx1i -O -K -N \
	-F+f32p,Times-Roman+jCB -X-3.5i >> $ps
gmt psscale -Dx3.25i/-0.4i+jTC+w5i/0.25i+h -C@ex_16.cpt -O >> $ps
#
rm -f *.nc gmt.conf
