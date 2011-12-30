REM		GMT EXAMPLE 20
REM
REM		$Id$
REM
REM Purpose:	Extend GMT to plot custom symbols
REM GMT progs:	pscoast, psxy
REM DOS calls:	del, echo
REM
echo GMT EXAMPLE 20
set ps=..\example_20.ps

REM Plot a world-map with volcano symbols of different sizes
REM on top given locations and sizes in hotspots.d

pscoast -Rg -JR9i -B60/30:."Hotspot Islands and Cities": -Gdarkgreen -Slightblue -Dc -A5000 -K -U"Example 20 in Cookbook" > %ps%

echo 55.5 -21.0 0.25 > hotspots.d
echo 63.0 -49.0 0.25 >> hotspots.d
echo -12.0 -37.0 0.25 >> hotspots.d
echo -28.5 29.34 0.25 >> hotspots.d
echo 48.4 -53.4 0.25 >> hotspots.d
echo 155.5 -40.4 0.25 >> hotspots.d
echo -155.5 19.6 0.5 >> hotspots.d
echo -138.1 -50.9 0.25 >> hotspots.d
echo -153.5 -21.0 0.25 >> hotspots.d
echo -116.7 -26.3 0.25 >> hotspots.d
echo -16.5 64.4 0.25 >> hotspots.d

psxy -R -J hotspots.d -Skvolcano -O -K -Wthinnest -Gred >> %ps%

REM Overlay a few bullseyes at NY, Cairo, and Perth

echo 286 40.45 0.8 > cities.d
echo 31.15 30.03 0.8 >> cities.d
echo 115.49 -31.58 0.8 >> cities.d

psxy -R -J cities.d -Skbullseye -O >> %ps%

del *.d
del .gmt*
