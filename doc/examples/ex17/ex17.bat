REM		GMT EXAMPLE 17
REM
REM Purpose:	Illustrates clipping of images using coastlines
REM GMT modules:	grd2cpt, grdimage, coast, text, makecpt
REM DOS calls:	del
REM
gmt begin ex17
	REM First generate geoid image w/ shading
	gmt grd2cpt @india_geoid.nc -Crainbow -H > geoid.cpt
	gmt grdimage @india_geoid.nc -I+d -JM16c -Cgeoid.cpt

	REM Then use gmt coast to initiate clip path for land
	gmt coast -R@india_geoid.nc -Dl -Gc

	REM Now generate topography image w/shading
	gmt makecpt -C150 -T-10000,10000 -N
	gmt grdimage @india_topo.nc -I+d

	REM Finally undo clipping and overlay basemap
	gmt coast -Q -B+t"Clipping of Images" -Ba10f5

	REM Put a colorbar on top of the land mask
	gmt colorbar -DjTR+o0.8c/0.2c+w10c/0.5c+h -Cgeoid.cpt -Bx5f1 -By+lm -I

	REM Add a text paragraph
	echo ^> 90 -10 12p 8c j > tmp.txt
	echo @_@%%5%%Example 17.@%%%%@_  We first plot the color geoid image >> tmp.txt
	echo for the entire region, followed by a gray-shaded @REMetopo5@REM >> tmp.txt
	echo image that is clipped so it is only visible inside the coastlines. >> tmp.txt
	gmt text tmp.txt -M -Gwhite -Wthinner -C+tO -D-8p/8p -F+f12,Times-Roman+jRB

	REM Clean up
	del geoid.cpt tmp.txt
gmt end show
