#!/usr/bin/env bash
#		GMT EXAMPLE 16
#
# Purpose:	Illustrates interpolation methods using same data as Example 12.
# GMT modules:	gmtset, grdview, grdfilter, contour, colorbar, surface, triangulate
# Unix progs:	rm
#
# Illustrate various means of contouring, using triangulate and surface.
# PW: Trouble with annotations and offset to title?
#
gmt begin ex16 ps
  gmt set FONT_ANNOT_PRIMARY 9p FONT_TITLE 18p,Times-Roman
  gmt colorbar -Dx3.25i/0i+jTC+w5i/0.25i+h -C@ex_16.cpt
  gmt subplot begin 2x2 -M0.05i -Fs3.25i/0 -R0/6.5/-0.2/6.5 -Jx1i -SCb -SRl+t -Bwesn -Yh+0.2i -T"Gridding of Data"
    gmt contour @Table_5_11.txt -C@ex_16.cpt -I -B+t"contour (triangulate)" -c1,1
    # 
    gmt surface @Table_5_11.txt -R0/6.5/-0.2/6.5 -I0.2 -Graws0.nc
    gmt grdview raws0.nc -C@ex_16.cpt -Qs -B+t"surface (tension = 0)" -c1,2
    #
    gmt surface @Table_5_11.txt -Graws5.nc -T0.5
    gmt grdview raws5.nc -C@ex_16.cpt -Qs -B+t"surface (tension = 0.5)" -c2,1
    #
    gmt triangulate @Table_5_11.txt -Grawt.nc
    gmt grdfilter rawt.nc -Gfiltered.nc -D0 -Fc1
    gmt grdview filtered.nc -C@ex_16.cpt -Qs -B+t"triangulate @~\256@~ grdfilter" -c2,2
  gmt subplot end
gmt end
rm -f raws0.nc raws5.nc rawt.nc filtered.nc
