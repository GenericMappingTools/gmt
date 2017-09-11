#!/bin/bash
#		GMT EXAMPLE 16
#
# Purpose:	Illustrates interpolation methods using same data as Example 12.
# GMT modules:	gmtset, grdview, grdfilter, pscontour, psscale, surface, triangulate
# Unix progs:	rm
#
# Illustrate various means of contouring, using triangulate and surface.
# PW: Trouble with annotations and offset to title?
#
#export GMT_PPID=$$
export GMT_PPID=1
gmt begin ex16 ps
  gmt set FONT_ANNOT_PRIMARY 9p FONT_TITLE 18p,Times-Roman
  gmt psscale -Dx3.25i/0i+jTC+w5i/0.25i+h -C@ex_16.cpt -P
  gmt subplot begin 2x2 -M0.05i -Fs3.25i/0+d -R0/6.5/-0.2/6.5 -Jx1i -SCb+t -SRl -Yh+0.4i -T"Gridding of Data"
    gmt pscontour @table_5.11 -C@ex_16.cpt -I -B+t"pscontour (triangulate)" -c1,1
    # 
    gmt surface @table_5.11 -R0/6.5/-0.2/6.5 -I0.2 -Graws0.nc
    gmt grdview raws0.nc -C@ex_16.cpt -Qs -B+t"surface (tension = 0)" -c1,2
    #
    gmt surface @table_5.11 -Graws5.nc -T0.5
    gmt grdview raws5.nc -C@ex_16.cpt -Qs -B+t"surface (tension = 0.5)" -c2,1
    #
    gmt triangulate @table_5.11 -Grawt.nc
    gmt grdfilter rawt.nc -Gfiltered.nc -D0 -Fc1
    gmt grdview filtered.nc -C@ex_16.cpt -Qs -B+t"triangulate @~\256@~ grdfilter" -c2,2
  gmt subplot end
gmt end
rm -f raws0.nc raws5.nc rawt.nc filtered.nc
