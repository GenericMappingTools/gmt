#!/usr/bin/env bash
#		GMT EXAMPLE 01
#
# Purpose:	Make two contour maps based on the data in the file osu91a1f_16.nc
# GMT modules:	set, subplot, grdcontour, coast
#
gmt begin ex01
	gmt set MAP_GRID_CROSS_SIZE_PRIMARY 0 FONT_ANNOT_PRIMARY 10p
	gmt subplot begin 2x1 -A -M0.25i -Blrtb -Bafg -T"Low Order Geoid" -Fs6.5i/0 -Rg -JH6.5i
		gmt coast -JH? -Glightbrown -Slightblue -c0,0
		gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd4i -Ln -Wcthinnest,- -Wathin,- -T+d0.1i/0.02i+l
		gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd4i -LP -T+d0.1i/0.02i+l

		gmt coast -JH0/? -Glightbrown -Slightblue -c1,0
		gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd4i -Ln -Wcthinnest,- -Wathin,- -T+d0.1i/0.02i
		gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd4i -LP -T+d0.1i/0.02i
	gmt subplot end
gmt end show
