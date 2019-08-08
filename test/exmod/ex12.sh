#!/usr/bin/env bash
#		GMT EXAMPLE 12
#
# Purpose:	Illustrates Delaunay triangulation of points, and contouring
# GMT modules:	makecpt, gmtinfo, contour, text, plot, triangulate, subplot
# Unix progs:	rm
#
gmt begin ex12 ps
  # Contour the data and draw triangles using dashed pen; use "gmt gmtinfo" and "gmt makecpt" to make a
  # color palette (.cpt) file
  T=`gmt info -T25+c2 @Table_5_11.txt`
  gmt makecpt -Cjet $T
  gmt subplot begin 2x2 -M0.05i -Fs3i/0 -SCb -SRl -R0/6.5/-0.2/6.5 -Jx3i -BWSne -T"Delaunay Triangulation"
  # First draw network and label the nodes
  gmt triangulate @Table_5_11.txt -M > net.xy
  gmt plot net.xy -Wthinner -c0,0
  gmt plot @Table_5_11.txt -Sc0.12i -Gwhite -Wthinnest
  gmt text @Table_5_11.txt -F+f6p+r
  # Then draw network and print the node values
  gmt plot net.xy -Wthinner -c0,1
  gmt plot @Table_5_11.txt -Sc0.03i -Gblack
  gmt text @Table_5_11.txt -F+f6p+jLM -Gwhite -W -C0.01i -D0.08i/0i -N
  gmt contour @Table_5_11.txt -Wthin -C -Lthinnest,- -Gd1i -c1,0
  # Finally color the topography
  gmt contour @Table_5_11.txt -C -I -c1,1
  gmt subplot end
gmt end
#
rm -f net.xy
