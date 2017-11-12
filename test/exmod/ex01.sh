#!/bin/bash
#		GMT EXAMPLE 01
#		$Id$
#
# Purpose:	Make two contour maps based on the data in the file osu91a1f_16.nc
# GMT modules:	set, subplot, basemap, grdcontour, coast
#
gmt begin ex01 ps
  gmt set MAP_GRID_CROSS_SIZE_PRIMARY 0 FONT_ANNOT_PRIMARY 10p PS_MEDIA letter
  gmt subplot begin 2x1 -A -M0.25i -Blrtb -Bafg -T"Low Order Geoid" -Fs6.5i/0 -Rg -JH0/6.5i
  gmt coast -Glightbrown -Slightblue -c1,1
  gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd4i -Ln -Wcthinnest,- -Wathin,- -T+d0.1i/0.02i
  gmt grdcontour osu91a1f_16.nc -C10 -A50+f7p -Gd4i -LP -T+d0.1i/0.02i+l
  gmt coast -Glightbrown -Slightblue -c2,1
  gmt grdcontour osu91a1f_16.nc -C10 -A50+f7p -Gd4i -Ln -Wcthinnest,- -Wathin,- -T+d0.1i/0.02i+l
  gmt grdcontour osu91a1f_16.nc -C10 -A50+f7p -Gd4i -LP -T+d0.1i/0.02i+l
  gmt subplot end
gmt end
