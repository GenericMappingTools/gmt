#!/usr/bin/env bash
#		GMT EXAMPLE 17
#
# Purpose:	Illustrates clipping of images using coastlines
# GMT modules:	grd2cpt, grdimage, coast, text, makecpt
# Unix progs:	rm
#
gmt begin ex17

	# First generate geoid image w/ shading
	gmt grd2cpt @india_geoid.nc -Crainbow -H > geoid.cpt
	gmt grdimage @india_geoid.nc -I+d -JM16c -Cgeoid.cpt

	# Then use gmt coast to initiate clip path for land
	gmt coast -R@india_geoid.nc -Dl -G

	# Now generate topography image w/shading
	gmt makecpt -C150 -T-10000,10000 -N
	gmt grdimage @india_topo.nc -I+d

	# Finally undo clipping and overlay basemap
	gmt coast -Q -B+t"Clipping of Images" -B

	# Put a colorbar on top of the land mask
	gmt colorbar -DjTR+o0.8c/0.2c+w10c/0.5c+h -Cgeoid.cpt -Bx5f1 -By+lm -I

	# Add a text paragraph
	gmt text -M -Gwhite -Wthinner -C+tO -Dj-8p/8p -F+f12,Times-Roman+jRB <<- END
	> 90 -10 12p 8c j
	@_@%5%Example 17.@%%@_  We first plot the color geoid image
	for the entire region, followed by a gray-shaded @#etopo5@#
	image that is clipped so it is only visible inside the coastlines.
	END

	# Clean up
	rm -f geoid.cpt
gmt end show
