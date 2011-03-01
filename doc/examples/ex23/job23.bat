REM
REM             GMT EXAMPLE 23
REM
REM             $Id: job23.bat,v 1.17 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:    Plot distances from Rome and draw shortest paths
REM
REM GMT progs:  grdmath, grdcontour, pscoast, psxy, pstext, grdtrack
REM DOS calls:  del
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

grdmath -Rg -I1 %lon% %lat% SDIST 111.13 MUL = dist.nc

REM Location info for 5 other cities + label justification

echo 105.87	21.02	HANOI		LM > cities.d
echo 282.95	-12.1	LIMA		LM >> cities.d
echo 178.42	-18.13	SUVA		LM >> cities.d
echo 237.67	47.58	SEATTLE		RM >> cities.d
echo 28.20	-25.75	PRETORIA	LM >> cities.d

pscoast -Rg -JH90/9i -Glightgreen -Sblue -U"Example 23 in Cookbook" -A1000 -B0g30:."Distances from %name% to the World": -K -Dc -Wthinnest > ..\example_23.ps

grdcontour dist.nc -A1000+v+ukm+kwhite -Glz-/z+ -S8 -C500 -O -K -J -Wathin,white -Wcthinnest,white,- >> ..\example_23.ps

REM For each of the cities, plot great circle arc to Rome with psxy

echo 105.87 21.02 > pts.d
echo %lon% %lat% >> pts.d
psxy -R -J -O -K -Wthickest/red pts.d >> ..\example_23.ps
echo 282.95 -12.1 > pts.d
echo %lon% %lat% >> pts.d
psxy -R -J -O -K -Wthickest/red pts.d >> ..\example_23.ps
echo 178.42 -18.13 > pts.d
echo %lon% %lat% >> pts.d
psxy -R -J -O -K -Wthickest/red pts.d >> ..\example_23.ps
echo 237.67 47.58 > pts.d
echo %lon% 	%lat% >> pts.d
psxy -R -J -O -K -Wthickest/red pts.d >> ..\example_23.ps
echo 28.20 -25.75 > pts.d
echo %lon% %lat% >> pts.d
psxy -R -J -O -K -Wthickest/red pts.d >> ..\example_23.ps

REM Plot red squares at cities and plot names:
psxy -R -J -O -K -Ss0.2 -Gred -Wthinnest cities.d >> ..\example_23.ps
echo {print $1, $2, 12, 0, 9, $4, $3} > awk.1
gawk -f awk.1 cities.d | pstext -R -J -O -K -Dj0.15/0 -Gred -N >> ..\example_23.ps
REM Place a yellow star at Rome
echo %lon% %lat% | psxy -R -J -O -K -Sa0.2i -Gyellow -Wthin >> ..\example_23.ps

REM Sample the distance grid at the cities and use the distance in km for labels

echo {printf "%%s %%s 12 0 1 CT %%d\n", $1, $2, int($NF+0.5)} > awk.2
grdtrack -Gdist.nc cities.d | gawk -f awk.2 > pts.d
pstext -R -J -O -D0/-0.2i -N -Wwhite,o -C0.02i/0.02i pts.d >> ..\example_23.ps

REM Clean up after ourselves:

del cities.d
del pts.d
del dist.nc
del .gmt*
del awk.*
if %master%==y cd ..
