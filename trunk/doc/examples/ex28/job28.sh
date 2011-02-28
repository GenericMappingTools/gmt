#!/bin/bash
#		GMT EXAMPLE 28
#		$Id: job28.sh,v 1.8 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Illustrates how to mix UTM data and UTM projection
# GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast, pstext, mapproject
# Unix progs:	rm, cut, grep, $AWK
#
. ../functions.sh
ps=../example_28.ps

# Get intensity grid and set up a color table
grdgradient Kilauea.utm.nc -Nt1 -A45 -GKilauea.utm_i.nc
makecpt -Ccopper -T0/1500/100 -Z > Kilauea.cpt
# Save min/max UTM coordinates with enough precision
grdinfo Kilauea.utm.nc --D_FORMAT=%.10g -C > tmp.txt
# Use inverse UTM projection to determine the lon/lat of the lower left and upper right corners
LL=`cut -f2,4 tmp.txt | mapproject -Ju5Q/1:1 -F -C -I --OUTPUT_DEGREE_FORMAT=ddd:mm:ss.x | \
	$AWK '{printf "%s/%s\n", $1, $2}'`
UR=`cut -f3,5 tmp.txt | mapproject -Ju5Q/1:1 -F -C -I --OUTPUT_DEGREE_FORMAT=ddd:mm:ss.x | \
	$AWK '{printf "%s/%s\n", $1, $2}'`
# Lay down the UTM topo grid using a 1:17,000 scale
grdimage Kilauea.utm.nc -IKilauea.utm_i.nc -CKilauea.cpt -Jx1:170000 -P -K -B5000g5000WSne \
	-U"Example 28 in Cookbook" --D_FORMAT=%.10g --ANNOT_FONT_SIZE_PRIMARY=9 \
	--GRID_CROSS_SIZE_PRIMARY=0.1i > $ps
# Overlay geographic data and coregister by using correct region and projection with the same scale
pscoast -R$LL/${UR}r -Ju5Q/1:170000 -O -K -Df+ -Slightblue -W0.5p -B5mg5mNE \
	--ANNOT_FONT_SIZE_PRIMARY=12 --PLOT_DEGREE_FORMAT=ddd:mmF >> $ps
psbasemap -R -J -O -K --ANNOT_FONT_SIZE_PRIMARY=9 -Lf155:07:30W/19:15:40N/19:23N/5k+l1:17,000+u \
	--LABEL_FONT_SIZE=10 >> $ps
echo 155:16:20W 19:26:20N 12 0 1 CB KILAUEA | pstext -R -J -O >> $ps
# Clean up

rm -f Kilauea.utm_i.nc Kilauea.cpt tmp.txt .gmt*
