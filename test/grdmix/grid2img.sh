#!/usr/bin/env bash
#
# Testing gmt grdmix on converting a grid to an image
# via a CPT w/wo shading.

gmt begin grid2img pdf
	gmt subplot begin 2x2 -Rd -Fs10c/5c -Scb -Srl -JQ10c -A+gwhite+p0.5p
		# Plain Earth relief map for comparison
		gmt subplot set 0 -A"Z+CPT"
		gmt grdimage @earth_relief_30m_p
		# Get intensities and plot those in gray
		gmt grdgradient @earth_relief_30m_p -A45 -Nte0.8 -Gint.nc
		gmt subplot set 1 -A"INT"
		gmt grdimage int.nc -Cgray
		# Create a tif directly from a grid and CPT
		gmt grdmix @earth_relief_30m_p -Cgeo -Gimg.tif
		gmt subplot set 2 -A"Z+CPT->TIF"
		gmt grdimage img.tif -fg
		# Throw in intensities then build tif
		gmt grdmix @earth_relief_30m_p -Cgeo -Iint.nc -Gimg.tif
		gmt subplot set 3 -A"Z+CPT+INT->TIF"
		gmt grdimage img.tif -fg
	gmt subplot end
gmt end show
