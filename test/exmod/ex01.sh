#!/bin/bash
#		GMT EXAMPLE 01
#		$Id$
#
# Purpose:	Make two contour maps based on the data in the file osu91a1f_16.nc
# GMT modules:	set, subplot, psbasemap, grdcontour, pscoast
#
export GMT_PPID=$$
gmt begin ex01 ps
  gmt set MAP_GRID_CROSS_SIZE_PRIMARY 0 FONT_ANNOT_PRIMARY 10p PS_MEDIA letter
  gmt subplot begin 2x1 -A -M0.25i -Lwesn+g -T"Low Order Geoid" -Fs6.5i/3.25i+p1p
  gmt pscoast -Rg -JH0 -Glightbrown -Slightblue -c1,1
  gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd4i -Ln -Wcthinnest,- -Wathin,- -T+d0.1i/0.02i
  gmt grdcontour osu91a1f_16.nc -C10 -A50+f7p -Gd4i -LP -T+d0.1i/0.02i+l
  gmt pscoast -Glightbrown -Slightblue -c2,1
  gmt grdcontour osu91a1f_16.nc -C10 -A50+f7p -Gd4i -Ln -Wcthinnest,- -Wathin,- -T+d0.1i/0.02i+l
  gmt grdcontour osu91a1f_16.nc -C10 -A50+f7p -Gd4i -LP -T+d0.1i/0.02i+l
  gmt subplot end
gmt end
