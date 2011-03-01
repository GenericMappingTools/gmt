REM		GMT EXAMPLE 17
REM
REM		$Id: job17.bat,v 1.12 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Illustrates clipping of images using coastlines
REM GMT progs:	grd2cpt, grdgradient, grdimage, pscoast, pstext
REM DOS calls:	del, echo
REM
echo GMT EXAMPLE 17
set master=y
if exist job17.bat set master=n
if %master%==y cd ex17

REM First generate geoid image w/ shading

grd2cpt india_geoid.nc -Crainbow > geoid.cpt
grdgradient india_geoid.nc -Nt1 -A45 -Gindia_geoid_i.nc
grdimage india_geoid.nc -Iindia_geoid_i.nc -JM6.5i -Cgeoid.cpt -P -K -U"Example 17 in Cookbook" > ..\example_17.ps

REM Then use pscoast to initiate clip path for land

pscoast -Rindia_geoid.nc -J -O -K -Dl -Gc >> ..\example_17.ps

REM Now generate topography image w/shading

echo -10000 150 10000 150 > gray.cpt
grdgradient india_topo.nc -Nt1 -A45 -Gindia_topo_i.nc
grdimage india_topo.nc -Iindia_topo_i.nc -J -Cgray.cpt -O -K >> ..\example_17.ps

REM Finally undo clipping and overlay basemap

pscoast -R -J -O -K -Q -B10f5:."Clipping of Images": >> ..\example_17.ps

REM Put a color legend on top of the land mask

psscale -D4i/7.6i/4i/0.2ih -Cgeoid.cpt -B5f1/:m: -I -O -K >> ..\example_17.ps

REM Add a text paragraph (Note double %% to get a single % in DOS)
echo # 90 -10 12 0 4 RB 12p 3i j > tmp
echo @_@%%5%%Example 17.@%%%%@_  We first plot the color geoid image >> tmp
echo for the entire region, followed by a gray-shaded @#etopo5@# >> tmp
echo image that is clipped so it is only visible inside the coastlines. >> tmp

pstext -R -J -O -m# -Wwhite,Othinner -D-0.1i/0.1i tmp >> ..\example_17.ps

REM Clean up

del *.cpt
del *_i.nc
del tmp
del .gmt*
if %master%==y cd ..
