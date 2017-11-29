REM
REM             GMT EXAMPLE 23
REM
REM             $Id$
REM
REM Purpose:    Plot distances from Rome and draw shortest paths
REM
REM GMT progs:  grdmath, grdcontour, pscoast, psxy, pstext, grdtrack
REM DOS calls:  del
REM
echo GMT EXAMPLE 23
set ps=example_23.ps

REM Position and name of central point:

set lon=12.50
set lat=41.99
set name=Rome

REM Calculate distances (km) to all points on a global 1x1 grid

gmt grdmath -Rg -I1 %lon% %lat% SDIST 111.13 MUL = dist.nc

REM Location info for 5 other cities + label justification

echo 105.87	21.02  LM HANOI	    > cities.txt
echo 282.95	-12.1  LM LIMA	   >> cities.txt
echo 178.42	-18.13 LM SUVA	   >> cities.txt
echo 237.67	47.58  RM SEATTLE  >> cities.txt
echo 28.20	-25.75 LM PRETORIA >> cities.txt

gmt pscoast -Rg -JH90/9i -Glightgreen -Sblue -A1000 -Bg30 -B+t"Distances from $name to the World" -K -Dc -Wthinnest > %ps%

gmt grdcontour dist.nc -A1000+v+ukm+fwhite -Glz-/z+ -S8 -C500 -O -K -J -Wathin,white -Wcthinnest,white,- >> %ps%

REM For each of the cities, plot great circle arc to Rome with gmt psxy
gmt psxy -R -J -O -K -Wthickest,red -Fr%lon%/%lat% cities.txt >> %ps%

REM Plot red squares at cities and plot names:
gmt psxy -R -J -O -K -Ss0.2 -Gred -Wthinnest cities.txt >> %ps%
gmt pstext -R -J -O -K -Dj0.15/0 -F+f12p,Courier-Bold,red+j -N cities.txt >> %ps%
REM Place a yellow star at Rome
echo %lon% %lat% | gmt psxy -R -J -O -K -Sa0.2i -Gyellow -Wthin >> %ps%

REM Sample the distance grid at the cities and use the distance in km for labels

gmt grdtrack -Gdist.nc cities.txt -o0-2 --FORMAT_FLOAT_OUT=0:%g,1:%g,2:%.0f | gmt pstext -R -J -O -D0/-0.2i -N -Gwhite -W -C0.02i -F+f12p,Helvetica-Bold+jCT >> %ps%

REM Clean up after ourselves:

del cities.txt
del dist.nc
del .gmt*
