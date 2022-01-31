#!/usr/bin/env bash
# Test DOT operator in grdmath to simulate sun/shade
# Sharpen the cos curve by taking sqrt and subtrack some ambient light
# DVC_TEST
gmt begin earth_shade ps
	gmt grdmath -Rg -I30m -rp 120W 15N DOT DUP ABS SQRT EXCH SIGN MUL 0.4 SUB = c.nc
	gmt grdmath -Rg -I30m -rp 120W 15N DOT ACOS R2D = a.nc
	gmt subplot begin 2x1 -M0  -Fs16c -Rg -JG200/-20/10c -Bafg -T"DOT operator in grdmath"
 		gmt grdcontour a.nc -C5 -A10 -GlZ-,Z+ -c0
 		gmt grdimage @earth_relief_30m -Ic.nc -c1 --COLOR_HSV_MAX_S=0 --COLOR_HSV_MIN_V=0
 	gmt subplot end
gmt end show
