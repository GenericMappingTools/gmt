#!/bin/bash
# For variable transparency TIF we plot pixels as squares with individual transparency
# Must add -Q to revert to original opaque blended image (color match the transparent
# colors but the pixels remain opaque).

gmt begin image_vartrans
	gmt set FONT_TAG 9p PS_MEDIA letter
	gmt subplot begin 4x1 -Rd -Fs12c/6c -JQ12c -Scb -Srl -A+gwhite+p0.25p -Bafg10 -X5c -Y1c
	# 1. Create and plot a continuously varying transparency grid (0-1)
	gmt grdmath -Rd -I01d -r Y 180 DIV 0.5 ADD = transparency.nc
	gmt subplot set 0 -A"FAKE VARIABLE TRANSPARENCIES"
	gmt grdcontour transparency.nc -C0.05 -A0.1
	# 2. Plot original day image
	gmt subplot set 1 -A"RGB IMAGE WITH NO TRANSPARENCY (OPAQUE)"
	gmt basemap
	echo 0 0 BACKGROUND | gmt text -F+f32p,1+a45
	gmt grdimage @earth_day_01d
	# 3. Achieve variable transparency via transparent squares as pixels
	gmt subplot set 2 -A"RGBA IMAGE WITH VARIABLE TRANSPARENCY AS SQUARES"
	gmt basemap
	echo 0 0 BACKGROUND | gmt text -F+f32p,1+a45
	# Plot 01d day image grid after adding variable transparency
	gmt grdmix @earth_day_01d -Atransparency.nc -C -Grgba.tif
	gmt grdimage rgba.tif
	# 4. Do variable transparency via image blending instead (opaque)
	gmt subplot set 3 -A"RGBA IMAGE WITH VARIABLE TRANSPARENCY AS OPAQUE BLEND"
	gmt basemap
	echo 0 0 BACKGROUND | gmt text -F+f32p,1+a45
	gmt grdimage rgba.tif -Q
	gmt subplot end
gmt end show
