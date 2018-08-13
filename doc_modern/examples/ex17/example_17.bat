@echo off
REM		GMT EXAMPLE 17
REM
REM
REM Purpose:	Illustrates clipping of images using coastlines
REM GMT progs:	grd2cpt, grdimage, pscoast, pstext
REM DOS calls:	del, echo
REM
echo GMT EXAMPLE 17
set ps=example_17.ps

REM First generate geoid image w/ shading

gmt grd2cpt @india_geoid.nc -Crainbow > geoid.cpt
gmt grdimage @india_geoid.nc -I+a45+nt1 -JM6.5i -Cgeoid.cpt -P -K > %ps%

REM Then use gmt pscoast to initiate clip path for land

gmt pscoast -R@india_geoid.nc -J -O -K -Dl -Gc >> %ps%

REM Now generate topography image w/shading

gmt makecpt -C150 -T-10000,10000 -N > shade.cpt
gmt grdimage @india_topo.nc -I+a45+nt1 -J -Cshade.cpt -O -K >> %ps%

REM Finally undo clipping and overlay basemap

gmt pscoast -R -J -O -K -Q -B10f5 -B+t"Clipping of Images" >> %ps%

REM Put a color legend on top of the land mask

gmt psscale -DjTR+o0.3i/0.1i+w4i/0.2i+h -R -J -Cgeoid.cpt -Bx5f1 -By+lm -I -O -K >> %ps%

REM Add a text paragraph (Note double %% to get a single % in DOS)
echo 62 | gawk "{printf \"%%c 90 -10 12p 3i j\n\", $1}" > tmp
echo @_@%%5%%Example 17.@%%%%@_  We first plot the color geoid image >> tmp
echo for the entire region, followed by a gray-shaded @#etopo5@# >> tmp
echo image that is clipped so it is only visible inside the coastlines. >> tmp

gmt pstext -R -J -O -M -Gwhite -Wthinner -TO -D-0.1i/0.1i tmp -F+f12,Times-Roman+jRB >> %ps%

REM Clean up

del *.cpt
del tmp
del .gmt*
