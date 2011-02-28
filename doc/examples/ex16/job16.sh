#!/bin/bash
#		GMT EXAMPLE 16
#		$Id: job16.sh,v 1.13 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Illustrates interpolation methods using same data as Example 12.
# GMT progs:	gmtset, grdview, grdfilter, pscontour, psscale, pstext, surface, triangulate
# Unix progs:	echo, rm
#
# Illustrate various means of contouring, using triangulate and surface.
#
. ../functions.sh
ps=../example_16.ps
gmtset ANNOT_FONT_SIZE_PRIMARY 9
#
pscontour -R0/6.5/-0.2/6.5 -Jx0.45i -P -K -Y5.5i -Ba2f1WSne table_5.11 -Cex16.cpt -I > $ps
echo "3.25 7 18 0 4 CB pscontour (triangulate)" | pstext -R -J -O -K -N >> $ps
#
surface table_5.11 -R -I0.2 -Graws0.nc
grdview raws0.nc -R -J -Ba2f1WSne -Cex16.cpt -Qs -O -K -X3.5i >> $ps
echo "3.25 7 18 0 4 CB surface (tension = 0)" | pstext -R -J -O -K -N >> $ps
#
surface table_5.11 -R -I0.2 -Graws5.nc -T0.5
grdview raws5.nc -R -J -Ba2f1WSne -Cex16.cpt -Qs -O -K -Y-3.75i -X-3.5i >> $ps
echo "3.25 7 18 0 4 CB surface (tension = 0.5)" | pstext -R -J -O -K -N >> $ps
#
triangulate table_5.11 -Grawt.nc -R -I0.2 > /dev/null
grdfilter rawt.nc -Gfiltered.nc -D0 -Fc1
grdview filtered.nc -R -J -Ba2f1WSne -Cex16.cpt -Qs -O -K -X3.5i >> $ps
echo "3.25 7 18 0 4 CB triangulate @~\256@~ grdfilter" | pstext -R -J -O -K -N >> $ps
echo "3.2125 7.5 32 0 4 CB Gridding of Data" | pstext -R0/10/0/10 -Jx1i -O -K -N -X-3.5i >> $ps
psscale -D3.25i/0.35i/5i/0.25ih -Cex16.cpt -O -U"Example 16 in Cookbook" -Y-0.75i >> $ps
#
rm -f *.nc .gmt*
