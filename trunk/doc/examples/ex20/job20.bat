REM		GMT EXAMPLE 20
REM
REM		$Id: job20.bat,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
REM
REM Purpose:	Extend GMT to plot custom symbols
REM GMT progs:	pscoast, psxy, mapproject
REM DOS calls:	del, echo, gawk
REM
echo GMT EXAMPLE 20
set master=y
if exist job20.bat set master=n
if %master%==y cd ex20
REM Make volcano- and bullseye-symbol awk-scripts using
REM make_symbol on the definition files volcano.def and
REM bullseye.def

gawk -f make_s~1 volcano.def > volcano.awk
gawk -f make_s~1 bullseye.def > bullseye.awk

REM Plot a world-map with volcano symbols of different sizes
REM on top given locations and sizes in hotspots.d

pscoast -R0/360/-90/90 -JR180/9i -B60/30:."Hotspot Islands and Cities": -G0/150/0 -S200/200/255 -Dc -A5000 -K -U"Example 20 in Cookbook" > example_20.ps

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

mapproject -R0/360/-90/90 -JR -Di hotspots.d | gawk -f volcano.awk | psxy -R0/9/0/9 -Jx1i -O -K -M -W0.25p -G255/0/0 -L >> example_20.ps

REM Overlay a few bullseyes at NY, Cairo, and Perth

echo 286 40.45 0.8 > cities.d
echo 31.15 30.03 0.8 >> cities.d
echo 115.49 -31.58 0.8 >> cities.d

mapproject -R0/360/-90/90 -JR -Di cities.d | gawk -f bullseye.awk | psxy -R0/9/0/9 -Jx -O -M >> example_20.ps

del *.awk
del *.d
del .gmt*
if %master%==y cd ..
