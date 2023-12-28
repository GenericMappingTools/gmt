#!/usr/bin/env bash
#
# Testing a simple grid with -Q options for transparency based on
# a computed color (via grid z-value and CPT) or a specific color.

gmt begin grdimage_Q_effects
	gmt set FONT_TAG 12p PS_MEDIA letter
	gmt grdmath -R0/100/0/50 -I1 -rp X 10 DIV FLOOR = stripes.nc
	gmt grdmath -Rstripes.nc X 100 DIV Y 50 DIV MUL 2 MUL 1 SUB = intensity.nc
	gmt makecpt -Chot -T0/10/1
	gmt subplot begin 4x1 -R0/100/0/50 -Fs15c/5c -JX15c/5c -Scb -Srl -A+gwhite+p0.25p -Bafg10 -X3c -Y3c
		# 1. Just plot the stripes grid with no other effects
		gmt subplot set 0 -A"PLAIN GRID and CPT [NO -Q]"
		gmt basemap
		echo 50 20 BACKGROUND | gmt text -F+f32p,1+a45
		gmt grdimage stripes.nc
		# 2. Plot stripes with intensity variations 
		gmt subplot set 1 -A"SAME PLUS LINEAR INTENSITY FROM LL (-1) TO UR (+1) [No -Q]"
		gmt basemap
		echo 50 20 BACKGROUND | gmt text -F+f32p,1+a45
		gmt grdimage stripes.nc -Iintensity.nc
		# 3. Plot stripes but set z = 5 to be transparent
		gmt subplot set 2 -A"PLAIN GRID WITH TRANSPARENCY FOR Z = 5 [-Q+z5]"
		gmt basemap
		echo 50 20 BACKGROUND | gmt text -F+f32p,1+a45
		gmt grdimage stripes.nc -Q+z5
		# 4. Plot stripes but set color equal yellow (z = 7-8) to be transparent
		gmt subplot set 3 -A"PLAIN GRID WITH TRANSPARENCY AT COLOR = yellow (z = 7-8) [-Qyellow]"
		gmt basemap
		echo 70 25 BACKGROUND | gmt text -F+f32p,1+a45
		gmt grdimage stripes.nc -Qyellow
	gmt subplot end
	# Place the colorbar beneath the subplot */
	gmt colorbar -DJBC
gmt end show
