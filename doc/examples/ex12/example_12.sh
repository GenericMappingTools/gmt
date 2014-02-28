#!/bin/bash
#		GMT EXAMPLE 12
#		$Id$
#
# Purpose:	Illustrates Delaunay triangulation of points, and contouring
# GMT progs:	makecpt, gmtinfo, pscontour, pstext, psxy, triangulate
# Unix progs:	$AWK, echo, rm
#
# First draw network and label the nodes
#
ps=example_12.ps
gmt triangulate table_5.11 -M > net.xy
gmt psxy -R0/6.5/-0.2/6.5 -JX3.06i/3.15i -B2f1 -BWSNe net.xy -Wthinner -P -K -X0.9i -Y4.65i > $ps
gmt psxy table_5.11 -R -J -O -K -Sc0.12i -Gwhite -Wthinnest >> $ps
$AWK '{print $1, $2, NR-1}' table_5.11 | gmt pstext -R -J -F+f6p -O -K >> $ps
#
# Then draw network and print the node values
#
gmt psxy -R -J -B2f1 -BeSNw net.xy -Wthinner -O -K -X3.25i >> $ps
gmt psxy -R -J -O -K table_5.11 -Sc0.03i -Gblack >> $ps
gmt pstext table_5.11 -R -J -F+f6p+jLM -O -K -Gwhite -W -C0.01i -D0.08i/0i -N >> $ps
#
# Then contour the data and draw triangles using dashed pen; use "gmt gmtinfo" and "gmt makecpt" to make a
# color palette (.cpt) file
#
T=`gmt info -T25/2 table_5.11`
gmt makecpt -Cjet $T > topo.cpt
gmt pscontour -R -J table_5.11 -B2f1 -BWSne -Wthin -Ctopo.cpt -Lthinnest,- -Gd1i -X-3.25i -Y-3.65i \
	-O -K >> $ps
#
# Finally color the topography
#
gmt pscontour -R -J table_5.11 -B2f1 -BeSnw -Ctopo.cpt -I -X3.25i -O -K >> $ps
echo "3.16 8 Delaunay Triangulation" | \
	gmt pstext -R0/8/0/11 -Jx1i -F+f30p,Helvetica-Bold+jCB -O -X-3.25i >> $ps
#
rm -f net.xy topo.cpt
