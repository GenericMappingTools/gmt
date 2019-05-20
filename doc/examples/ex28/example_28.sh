#!/usr/bin/env bash
#		GMT EXAMPLE 28
#
# Purpose:	Illustrates how to mix UTM data and UTM gmt projection
# GMT modules:	makecpt, grdimage, grdmath, pscoast, pstext,
# Unix progs:	rm, echo
#
ps=example_28.ps

# Set up a color table
gmt makecpt -Ccopper -T0/1500 > Kilauea.cpt
# Lay down the UTM topo grid using a 1:160,000 scale
gmt grdimage @Kilauea.utm.nc -I+d -CKilauea.cpt -Jx1:160000 -P -K \
	--FONT_ANNOT_PRIMARY=9p > $ps
# Overlay geographic data and coregister by using correct region and gmt projection with the same scale
gmt pscoast -R@Kilauea.utm.nc -Ju5Q/1:160000 -O -K -Df+ -Slightblue -W0.5p -B5mg5m -BNE \
	--FONT_ANNOT_PRIMARY=12p --FORMAT_GEO_MAP=ddd:mmF >> $ps
echo 155:16:20W 19:26:20N KILAUEA | gmt pstext -R -J -O -K -F+f12p,Helvetica-Bold+jCB >> $ps
gmt psbasemap -R -J -O -K --FONT_ANNOT_PRIMARY=9p -LjRB+c19:23N+f+w5k+l1:160,000+u+o0.2i \
	--FONT_LABEL=10p >> $ps
# Annotate in km but append ,000m to annotations to get customized meter labels
gmt psbasemap -R@Kilauea.utm.nc+Uk -Jx1:160 -B5g5+u"@:8:000m@::" -BWSne -O --FONT_ANNOT_PRIMARY=10p \
	--MAP_GRID_CROSS_SIZE_PRIMARY=0.1i --FONT_LABEL=10p >> $ps
# Clean up
rm -f Kilauea.cpt tmp.txt
