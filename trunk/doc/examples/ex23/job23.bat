REM
REM		GMT EXAMPLE 23
REM
REM		$Id: job23.bat,v 1.3 2004-04-28 19:09:24 pwessel Exp $
REM
REM Purpose:	Plot distances from Rome and draw shortest paths
REM GMT progs:	gmtset, grdmath, grdcontour, psxy, pstext, grdtrack
REM DOS calls:	echo, gawk
REM
echo GMT EXAMPLE 23
set master=y
if exist job23.bat set master=n
if %master%==y cd ex23

REM Position and name of central point:

set lon=12.50
set lat=41.99
set name=Rome

REM Calculate distances (km) to all points on a global 1x1 grid

grdmath -Rg -I1 %lon% %lat% SDIST 111.13 MUL = dist.grd

REM Location info for 5 other cities

echo 139.75	35.67	TOKYO > cities.d
echo 282.95	-12.1	LIMA >> cities.d
echo 151.17	-33.92	SYDNEY >> cities.d
echo 237.67	47.58	SEATTLE >> cities.d
echo 28.03	-26.17	JOHANNESBURG >> cities.d

pscoast -Rg -JH90/9i -Glightgreen -Sblue -U"Example 23 in Cookbook" -A1000 -B0g30:."Distances from %name% to the World": -K -Dc -Wthinnest > example_23.ps

REM Temporarily switch basemap pen in order to annotate white contours

gmtset BASEMAP_FRAME_RGB white
grdcontour dist.grd -A1000t -C500 -O -K -J -Wathin,white -Wcthinnest,- >> example_23.ps
gmtset BASEMAP_FRAME_RGB black

REM Find the number of cities:

set n=5

REM For each of these cities, plot great circle arc with psxy


REM Plot red squares at cities and plot names:
psxy -R -J -O -K -Ss0.2 -Gred -W0.25p cities.d >> example_23.ps
gawk "{print $1, $2, 12, 1, 9, \"LM\", $3}" cities.d | pstext -R -J -O -K -D0.15/0 -Gred -N >> example_23.ps
REM Place a yellow star at Rome
echo %lon% %lat% | psxy -R -J -O -K -Sa0.2i -Gyellow -Wthin >> example_23.ps

REM Sample the distance grid at the cities and use the distance in km for labels

grdtrack -Gdist.grd cities.d | gawk "{printf \"%s %s 12 0 1 CB %d\n\", $1, $2, int($NF+0.5)}" | pstext -R -J -O -D0/0.2i -N -Wwhiteo -C0.02i/0.02i >> example_23.ps

REM Clean up after ourselves:

del cities.d
del dist.grd
del .gmt*
if %master%==y cd ..
