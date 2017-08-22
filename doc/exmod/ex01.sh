#!/bin/bash
#		GMT EXAMPLE 01
#		$Id$
#
# Purpose:	Make two contour maps based on the data in the file osu91a1f_16.nc
# GMT modules:	set, subplot, grdcontour, psbasemap, pscoast
# Unix progs:	rm
#
gmt begin ex01 ps
  gmt set MAP_GRID_CROSS_SIZE_PRIMARY 0 FONT_ANNOT_PRIMARY 10p PS_MEDIA letter
  gmt subplot begin 2x1 -A -Mm1i -Mp0.25i -L -T"Low Order Geoid" -F8.5i/9.618i+p1p+d -Vd
  gmt pscoast -Rg -JH0 -Bg30 -Glightbrown -Slightblue -c1,1
  gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd4i -Ln -Wcthinnest,- -Wathin,- -T+d0.1i/0.02i
  gmt grdcontour osu91a1f_16.nc -C10 -A50+f7p -Gd4i -LP -T+d0.1i/0.02i
  gmt pscoast -Bg30 -Glightbrown -Slightblue -c2,1
  gmt grdcontour osu91a1f_16.nc -C10 -A50+f7p -Gd4i -Ln -Wcthinnest,- -Wathin,- -T+d0.1i/0.02i+l
  gmt grdcontour osu91a1f_16.nc -C10 -A50+f7p -Gd4i -LP -T+d0.1i/0.02i+l
  gmt subplot end
gmt end
