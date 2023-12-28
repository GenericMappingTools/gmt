#!/usr/bin/env bash
#
# Testing gmt grdmix on adding A to RGB image and plot via grdimage

gmt begin transp_mix pdf
	gmt set MAP_GRID_PEN 2p FONT_TAG 9p PS_MEDIA letter PS_PAGE_ORIENTATION portrait
	gmt grdmath -Rd -I01d -rp X 360 DIV 0.5 ADD Y 180 DIV 0.5 ADD MUL = transparency.nc
	gmt subplot begin 3x2 -Rd -Fs9c/4.5c -JQ9c -Scb -Srl -A+gwhite+p0.25p -Bafg30 -X2c
		# 1. Plot 01d day image on top of grid lines (which will be invisible)
		gmt subplot set 0 -A"JUST IMAGE (OPAQUE)"
		gmt basemap
		gmt grdimage @earth_day_01d_p
		# 2. Plot the fake transparencies
		gmt subplot set 1 -A"FAKE TRANSPARENCY (OPAQUE)"
		gmt basemap
		gmt grdimage transparency.nc -Chot
		# 3. Ignore transparencies when making the tif and plot the image on top of grid lines (which will be invisible)
		gmt grdmix @earth_day_01d_p -C -Grgba.tif
		gmt subplot set 2 -A"IMAGE NO TRANSPARENCY (OPAQUE)"
		gmt basemap
		gmt grdimage rgba.tif
		# 4. Mix transparencies into the tif and plot the image on top of grid lines (which will be invisible)
		gmt grdmix @earth_day_01d_p -Atransparency.nc -C -Grgba.tif
		gmt subplot set 3 -A"BLENDED TRANSPARENT IMAGE (OPAQUE)"
		gmt basemap
		gmt grdimage rgba.tif -Qwhite
		# 5. Plot image using -Q+i to invert opacity to transparency on top of grid lines (which will be invisible)
		gmt subplot set 4 -A"BLENDED OPACITY IMAGE (OPAQUE)"
		gmt basemap
		gmt grdimage rgba.tif -Q+i
		# 6. Plot image using defaults on top of grid lines (which will visible through the squares)
		gmt subplot set 5 -A"DEFAULT IMAGE VIA SQUARES (TRUE TRANSPARENCY)"
		gmt basemap
		gmt grdimage rgba.tif
	gmt subplot end
gmt end show
