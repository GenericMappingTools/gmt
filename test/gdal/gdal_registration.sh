#!/usr/bin/env bash
#
# Test that the node registration of a grid survives a round-trip through a GDAL format.
#
# GDAL only records the AREA_OR_POINT tag in the file when it is "Point", since "Area" is the
# implicit default, and it writes no GeoTIFF keys at all for a raster that carries no referencing.
# Because of that a pixel registered Cartesian grid used to come back gridline registered, with a
# half-cell shifted region.  See issues #8633 (grdsample) and #8988 (grdmath).

cat << EOF > answer.txt
pixel.tif 0 10 0 10 1
gridline.tif 0 10 0 10 0
lonlat.tif -180 180 -90 90 1
math.tif 0 1 0 1 1
pixel.asc 0 10 0 10 1
EOF

# Cartesian gridline grid to start from
gmt grdmath -R0/10/0/10 -I1 X Y MUL = gridline.nc

gmt grdsample gridline.nc -r -Gpixel.tif=gd:GTiff		# -r must give a pixel registered GeoTIFF
gmt grdconvert gridline.nc -Ggridline.tif=gd:GTiff		# and a gridline grid must stay gridline
gmt grdmath -R-180/180/-90/90 -I2 -rp -fg X = lonlat.tif=gd:GTiff	# Referenced grids worked before
gmt grdmath -R0/1/0/1 -I0.1 -rp X = "math.tif=gd:GTiff+cCOMPRESS=LZW"	# Also with +c<options>
gmt grdsample gridline.nc -r -Gpixel.asc=gd:AAIGrid		# Not only the GeoTIFF driver

for file in pixel.tif gridline.tif lonlat.tif math.tif pixel.asc; do
	gmt grdinfo ${file} -C | awk '{printf "%s %s %s %s %s %s\n", $1, $2, $3, $4, $5, $12}'
done > result.txt

diff -q --strip-trailing-cr answer.txt result.txt
