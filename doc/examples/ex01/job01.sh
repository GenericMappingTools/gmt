#!/bin/bash
#		GMT EXAMPLE 01
#		$Id: job01.sh,v 1.17 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Make two contour maps based on the data in the file osu91a1f_16.nc
# GMT progs:	gmtset, grdcontour, psbasemap, pscoast
# Unix progs:	rm
#
. ../functions.sh
ps=../example_01.ps
gmtset GRID_CROSS_SIZE_PRIMARY 0 ANNOT_FONT_SIZE_PRIMARY 10
psbasemap -R0/6.5/0/9 -Jx1i -B0 -P -K -U"Example 1 in Cookbook" > $ps
pscoast -Rg -JH0/6i -X0.25i -Y0.5i -O -K -Bg30 -Dc -Glightgray >> $ps
grdcontour osu91a1f_16.nc -J -C10 -A50+s7 -Gd4i -L-1000/-1 -Wcthinnest,- -Wathin,- -O -K \
	-T0.1i/0.02i >> $ps
grdcontour osu91a1f_16.nc -J -C10 -A50+s7 -Gd4i -L-1/1000 -O -K -T0.1i/0.02i >> $ps
pscoast -Rg -JH6i -Y4i -O -K -Bg30:."Low Order Geoid": -Dc -Glightgray >> $ps
grdcontour osu91a1f_16.nc -J -C10 -A50+s7 -Gd4i -L-1000/-1 -Wcthinnest,- -Wathin,- -O -K \
	-T0.1i/0.02i:-+ >> $ps
grdcontour osu91a1f_16.nc -J -C10 -A50+s7 -Gd4i -L-1/1000 -O -T0.1i/0.02i:-+ >> $ps
rm -f .gmt*
