#!/usr/bin/env bash
#
# Testing various grids and transparency in grdimage
# Top row is a topo relief grid (image via CPT) and the same grid with fake NaNs to be transparent

gmt begin grid_transp
	gmt set FONT_TAG 12p PS_MEDIA letter
	# Make two grids: one a 0|1 multiply grid setting a region to zero and one making NaNs over Africa and 1 elsewhere
	gmt grdmath -Rd -I01d -rp -fg 1 X -40 SUB ABS 40 LT Y -20 SUB ABS 20 LT MUL SUB = zero.nc
	gmt grdmath -Rzero.nc 0 0 SDIST 4000 GT 0 NAN = nan.nc
	gmt subplot begin 4x1 -Rd -Fs12c/6c -JQ12c -Scb -Srl -A+gwhite+p0.25p -Bafg10 -X5c -Y1c
		# 1. Plot 01d relief grid on top of grid lines (which will be invisible)
		gmt subplot set 0 -A"TOPOGRAPHY GRID WITH NO NANS"
		gmt basemap
		echo 0 0 BACKGROUND | gmt text -F+f32p,1+a45
		gmt grdimage @earth_relief_01d_p
		# 2. Plot 01d relief grid after setting a R=4000 chunk around (0,0) to NaNs
		gmt subplot set 1 -A"TOPOGRAPHY GRID WITH NANS AS MISSING DATA"
		gmt grdmath nan.nc @earth_relief_01d_p MUL = hurt.nc
		gmt basemap
		echo 0 0 BACKGROUND | gmt text -F+f32p,1+a45
		gmt grdimage hurt.nc -Cgeo
		# 3. Plot the same grid but now say NaNs should be transparent
		gmt subplot set 2 -A"TOPO WITH NANS INDICATING FULL TRANSPARENCY"
		gmt basemap
		echo 0 0 BACKGROUND | gmt text -F+f32p,1+a45
		gmt grdimage hurt.nc -Cgeo -Q
		# 4. Plot 01d relief grid after setting a big chunk to zero and say zero is transparent
		gmt grdmath zero.nc @earth_relief_01d_p MUL = hurt.nc
		gmt subplot set 3 -A"TOPO WITH ZEROS INDICATING FULL TRANSPARENCY"
		gmt basemap
		echo 0 0 BACKGROUND | gmt text -F+f32p,1+a45
		gmt grdimage hurt.nc -Cgeo -Q+z0
	gmt subplot end
gmt end show
