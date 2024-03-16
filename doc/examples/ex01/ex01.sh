#!/usr/bin/env bash
#		GMT EXAMPLE 01
#
# Purpose:	Make two contour maps based on the data in the file osu91a1f_16.nc
# GMT modules:	subplot, grdcontour, coast
#
gmt begin ex01
	gmt subplot begin 2x1 -A -M0.5c -Blrtb -Bafg -T"Low Order Geoid" -Fs16c/0 -Rg -JH16c
		gmt coast -JH180/? -Glightbrown -Slightblue -c0,0
		gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd10c -Ln -Wcthinnest,- -Wathin,- -T+d8p/2p+l
		gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd10c -LP -T+d8p/2p+l

		gmt coast -JH0/? -Glightbrown -Slightblue -c1,0
		gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd10c -Ln -Wcthinnest,- -Wathin,- -T+d8p/2p
		gmt grdcontour @osu91a1f_16.nc -C10 -A50+f7p -Gd10c -LP -T+d8p/2p
	gmt subplot end
gmt end show
