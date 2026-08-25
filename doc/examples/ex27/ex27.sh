#!/usr/bin/env bash
#		GMT EXAMPLE 27
#
# Purpose:	Illustrates how to plot Mercator img grids
# GMT modules:	makecpt, grdimage, grdinfo, coast, colorbar
#
gmt begin ex27
	# Gravity in tasman_grav.nc is in 0.1 mGal increments and the grid
	# is already in projected Mercator x/y units.

	# Make a suitable cpt file for mGal
	gmt makecpt -T-120/120 -Crainbow

	# Since this is a Mercator grid we use a linear projection
	gmt grdimage @tasman_grav.nc=ns+s0.1 -I+d -Jx0.6c

	# Then use gmt coast to plot land; get original -R from grid img remark
	# and use Mercator gmt projection with same scale as above on a spherical Earth
	R=$(gmt grdinfo @tasman_grav.nc -Ii)
	gmt coast $R -Jm0.6c -B -BWSne -Gblack --PROJ_ELLIPSOID=Sphere -Cwhite -Dh+ --FORMAT_GEO_MAP=dddF

	# Put a color legend in top-left corner of the land mask
	gmt colorbar -DjTL+o1c+w5c/0.4c -Bx -By+lmGal -I -F+gwhite+p1p
gmt end show
