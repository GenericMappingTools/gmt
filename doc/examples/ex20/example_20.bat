REM		GMT EXAMPLE 20
REM
REM		$Id$
REM
REM Purpose:	Extend GMT to plot custom symbols
REM GMT progs:	pscoast, psxy
REM DOS calls:	del, echo
REM
echo GMT EXAMPLE 20
set ps=example_20.ps

REM Plot a world-map with volcano symbols of different sizes at hotspot locations
REM using table from Muller et al., 1993, Geology.

gmt pscoast -Rg -JR9i -Bx60 -By30 -B+t"Hotspot Islands and Hot Cities" -Gdarkgreen -Slightblue -Dc -A5000 -K > %ps%

gmt psxy -R -J @hotspots.txt -Skvolcano -O -K -Wthinnest -Gred >> %ps%

REM Overlay a few bullseyes at NY, Cairo, erth, and Montevideo

echo 74W 40.45N 0.5 > cities.txt
echo 31.15E 30.03N 0.5 >> cities.txt
echo 115.49E 31.58S 0.5 >> cities.txt
echo 56.16W 34.9S 0.5 >> cities.txt

gmt psxy -R -J cities.txt -Sk@bullseye -O >> %ps%

del cities.txt
del .gmt*
