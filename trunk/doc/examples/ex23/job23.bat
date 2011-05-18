REM
REM             GMT EXAMPLE 23
REM
REM             $Id: job23.bat,v 1.19 2011-05-18 16:24:14 remko Exp $
REM
REM Purpose:    Plot distances from Rome and draw shortest paths
REM
REM GMT progs:  grdmath, grdcontour, pscoast, psxy, pstext, grdtrack
REM DOS calls:  del
REM
echo GMT EXAMPLE 23
set ps=..\example_23.ps

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

pscoast -Rg -JH90/9i -Glightgreen -Sblue -U"Example 23 in Cookbook" -A1000 -Bg30:."Distances from %name% to the World": -K -Dc -Wthinnest > %ps%

grdcontour dist.nc -A1000+v+ukm+kwhite -Glz-/z+ -S8 -C500 -O -K -J -Wathin,white -Wcthinnest,white,- >> %ps%

REM For each of the cities, plot great circle arc to Rome with psxy

echo 105.87 21.02 > pts.d
echo %lon% %lat% >> pts.d
psxy -R -J -O -K -Wthickest,red pts.d >> %ps%
echo 282.95 -12.1 > pts.d
echo %lon% %lat% >> pts.d
psxy -R -J -O -K -Wthickest,red pts.d >> %ps%
echo 178.42 -18.13 > pts.d
echo %lon% %lat% >> pts.d
psxy -R -J -O -K -Wthickest,red pts.d >> %ps%
echo 237.67 47.58 > pts.d
echo %lon% 	%lat% >> pts.d
psxy -R -J -O -K -Wthickest,red pts.d >> %ps%
echo 28.20 -25.75 > pts.d
echo %lon% %lat% >> pts.d
psxy -R -J -O -K -Wthickest,red pts.d >> %ps%

REM Plot red squares at cities and plot names:
psxy -R -J -O -K -Ss0.2 -Gred -Wthinnest cities.d >> %ps%
echo {print $1, $2, $4, $3} > awk.1
gawk -f awk.1 cities.d | pstext -R -J -O -K -Dj0.15/0 -F+f12p,Courier-Bold,red+j -N >> %ps%
REM Place a yellow star at Rome
echo %lon% %lat% | psxy -R -J -O -K -Sa0.2i -Gyellow -Wthin >> %ps%

REM Sample the distance grid at the cities and use the distance in km for labels

echo {printf "%%s %%s %%d\n", $1, $2, int($NF+0.5)} > awk.2
grdtrack -Gdist.nc cities.d | gawk -f awk.2 > pts.d
pstext -R -J -O -D0/-0.2i -N -Gwhite -W -C0.02i pts.d -F+f12p,Helvetica-Bold+jCT >> %ps%

REM Clean up after ourselves:

del cities.d
del pts.d
del dist.nc
del .gmt*
del awk.*
