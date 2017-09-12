#!/bin/bash
#		GMT EXAMPLE 12
#		$Id$
#
# Purpose:	Illustrates Delaunay triangulation of points, and contouring
# GMT modules:	makecpt, gmtinfo, pscontour, pstext, psxy, triangulate, subplot
# Unix progs:	rm
#
gmt begin ex12 ps
  gmt subplot begin 2x2 -M0.05i -Fs3i/0 -SCb -SRl -R0/6.5/-0.2/6.5 -Jx3i -BWSne -T"Delaunay Triangulation"
  # First draw network and label the nodes
  gmt triangulate @table_5.11 -M > net.xy
  gmt psxy net.xy -Wthinner -P -c1,1
  gmt psxy @table_5.11 -Sc0.12i -Gwhite -Wthinnest
  gmt pstext @table_5.11 -F+f6p+r
  # Then draw network and print the node values
  gmt psxy net.xy -Wthinner -c1,2
  gmt psxy @table_5.11 -Sc0.03i -Gblack
  gmt pstext @table_5.11 -F+f6p+jLM -Gwhite -W -C0.01i -D0.08i/0i -N
  # Then contour the data and draw triangles using dashed pen; use "gmt gmtinfo" and "gmt makecpt" to make a
  # color palette (.cpt) file
  T=`gmt info -T25+c2 @table_5.11`
  gmt makecpt -Cjet $T > topo.cpt
  gmt pscontour @table_5.11 -Wthin -Ctopo.cpt -Lthinnest,- -Gd1i -c2,1
  # Finally color the topography
  gmt pscontour @table_5.11 -Ctopo.cpt -I -c2,2
  gmt subplot end
gmt end
#
rm -f net.xy topo.cpt
