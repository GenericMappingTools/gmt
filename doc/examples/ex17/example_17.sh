#!/bin/bash
#		GMT EXAMPLE 17
#		$Id$
#
# Purpose:	Illustrates clipping of images using coastlines
# GMT progs:	grd2cpt, grdgradient, grdimage, pscoast, pstext
# Unix progs:	rm
#
ps=example_17.ps

# First generate geoid image w/ shading

gmt grd2cpt india_geoid.nc -Crainbow > geoid.cpt
gmt grdgradient india_geoid.nc -Nt1 -A45 -Gindia_geoid_i.nc
gmt grdimage india_geoid.nc -Iindia_geoid_i.nc -JM6.5i -Cgeoid.cpt -P -K > $ps

# Then use gmt pscoast to initiate clip path for land

gmt pscoast -Rindia_geoid.nc -J -O -K -Dl -Gc >> $ps

# Now generate topography image w/shading

echo "-10000 150 10000 150" > gray.cpt
gmt grdgradient india_topo.nc -Nt1 -A45 -Gindia_topo_i.nc
gmt grdimage india_topo.nc -Iindia_topo_i.nc -J -Cgray.cpt -O -K >> $ps

# Finally undo clipping and overlay basemap

gmt pscoast -R -J -O -K -Q -B10f5 -B+t"Clipping of Images" >> $ps

# Put a color legend on top of the land mask

gmt psscale -DjTR+o0.2+w4i/0.2i+h -R -J -Cgeoid.cpt -Bx5f1 -By+lm -I -O -K >> $ps

# Add a text paragraph

gmt pstext -R -J -O -M -Gwhite -Wthinner -TO -D-0.1i/0.1i -F+f12,Times-Roman+jRB >> $ps << END
> 90 -10 12p 3i j
@_@%5%Example 17.@%%@_  We first plot the color geoid image
for the entire region, followed by a gray-shaded @#etopo5@#
image that is clipped so it is only visible inside the coastlines.
END

# Clean up

rm -f geoid.cpt gray.cpt *_i.nc
