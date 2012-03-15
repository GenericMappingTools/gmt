#!/bin/bash
#		GMT EXAMPLE 28
#		$Id$
#
# Purpose:	Illustrates how to mix UTM data and UTM projection
# GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast, pstext, mapproject
# Unix progs:	rm, echo, $AWK
#
ps=example_28.ps

# Get intensity grid and set up a color table
grdgradient Kilauea.utm.nc -Nt1 -A45 -GKilauea.utm_i.nc
makecpt -Ccopper -T0/1500/100 -Z > Kilauea.cpt
# Save min/max UTM coordinates with enough precision
grdinfo Kilauea.utm.nc --FORMAT_FLOAT_OUT=%.10g -C > tmp.txt
# Use inverse UTM projection to determine the lon/lat of the lower left and upper right corners
LL=`mapproject tmp.txt -Ju5Q/1:1 -F -C -I --FORMAT_GEO_OUT=ddd:mm:ss.x -i1,3 -o0,1 | \
	$AWK '{printf "%s/%s\n", $1, $2}'`
UR=`mapproject tmp.txt -Ju5Q/1:1 -F -C -I --FORMAT_GEO_OUT=ddd:mm:ss.x -i2,4 -o0,1 | \
	$AWK '{printf "%s/%s\n", $1, $2}'`
# Lay down the UTM topo grid using a 1:16,000 scale
grdimage Kilauea.utm.nc -IKilauea.utm_i.nc -CKilauea.cpt -Jx1:160000 -P -K \
	-U"Example 28 in Cookbook" --FORMAT_FLOAT_OUT=%.10g --FONT_ANNOT_PRIMARY=9p \
	> $ps
# Overlay geographic data and coregister by using correct region and projection with the same scale
pscoast -R${LL}/${UR}r -Ju5Q/1:160000 -O -K -Df+ -Slightblue -W0.5p -B5mg5mNE \
	--FONT_ANNOT_PRIMARY=12p --FORMAT_GEO_MAP=ddd:mmF >> $ps
echo 155:16:20W 19:26:20N KILAUEA | pstext -R -J -O -K -F+f12p,Helvetica-Bold+jCB >> $ps
psbasemap -R -J -O -K --FONT_ANNOT_PRIMARY=9p -Lf155:07:30W/19:15:40N/19:23N/5k+l1:16,000+u \
	--FONT_LABEL=10p >> $ps
# Annotate in km but append ,000m to annotations to get customized meter labels
Rkm=`grdinfo Kilauea.utm.nc -C | cut -f2,3,4,5 | gmtconvert -i0-3s0.001 --IO_COL_SEPARATOR=/`
psbasemap -R$Rkm -Jx1:160 -B5g5:,"-@:8:000m":WSne -O --FONT_ANNOT_PRIMARY=10p \
	--MAP_GRID_CROSS_SIZE_PRIMARY=0.1i --FONT_LABEL=10p >> $ps
# Clean up
rm -f Kilauea.utm_i.nc Kilauea.cpt tmp.txt
