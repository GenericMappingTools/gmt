REM
REM		GMT EXAMPLE 22
REM
REM		$Id: job22.bat,v 1.4 2004-04-28 19:09:24 pwessel Exp $
REM
REM Purpose:	Automatic map of last 7 days of world-wide seismicity
REM GMT progs:	gmtset, pscoast, psxy, pslegend
REM DOS calls:	echo, gawk, wget
REM
echo GMT EXAMPLE 22
set master=y
if exist job22.bat set master=n
if %master%==y cd ex22
gmtset ANNOT_FONT_SIZE 10p HEADER_FONT_SIZE 18p PLOT_DEGREE_FORMAT ddd:mm:ssF

REM Get the data (-q quietly) from USGS using the wget (comment out in case
REM your system does not have wget)

REM wget http://neic.usgs.gov/neis/gis/bulletin.asc -q -O neic_quakes.d

REM Count the number of events (to be used in title later. one less due to header)
REM Pull out the first and last timestamp to use in legend title

gawk -F, "{if (NR == 2) { d = $1; t = $2} else { n++ }}; END {printf \"H 16 1 %d events during %s %s\", n, d, t}" neic_quakes.d > neis.legend
gawk -F, "{d = $1; t = $2}; END {printf \" to %s %s\n\", d, t}" neic_quakes.d >> neis.legend

REM Assign a string that contains the current user @ the current computer node.
REM Note that two @@ is needed to print a single @ in pstext:

set me="GMT guru @@ GMTbox"

REM Create standard seismicity color table

echo 0	red	100	red > neis.cpt
echo 100	green	300	green >> neis.cpt
echo 300	blue	10000	blue >> neis.cpt

REM Start plotting. First lay down map, then plot quakes with size = magintude/50":

pscoast -Rg -JK180/9i -B45g30:."World-wide earthquake activity": -Glightbrown -Slightblue -Dc -A1000 -K -U/-0.75i/-2.5i/"Example 22 in Cookbook" -Y2.75i > example_22.ps
gawk -F, "{ print $4, $3, $6, $5*0.02}" neic_quakes.d | psxy -R -JK -O -K -Cneis.cpt -Sci -Wthin -H >> example_22.ps

REM Create legend input file for NEIS quake plot

echo D 0 1p >> neis.legend
echo N 3 >> neis.legend
echo V 0 1p >> neis.legend
echo S 0.1i c 0.1i red 0.25p 0.2i Shallow depth (0-100 km) >> neis.legend
echo S 0.1i c 0.1i green 0.25p 0.2i Intermediate depth (100-300 km) >> neis.legend
echo S 0.1i c 0.1i blue 0.25p 0.2i Very deep (> 300 km) >> neis.legend
echo V 0 1p >> neis.legend
echo D 0 1p >> neis.legend
echo N 7 >> neis.legend
echo V 0 1p >> neis.legend
echo S 0.1i c 0.06i - 0.25p 0.3i M 3 >> neis.legend
echo S 0.1i c 0.08i - 0.25p 0.3i M 4 >> neis.legend
echo S 0.1i c 0.10i - 0.25p 0.3i M 5 >> neis.legend
echo S 0.1i c 0.12i - 0.25p 0.3i M 6 >> neis.legend
echo S 0.1i c 0.14i - 0.25p 0.3i M 7 >> neis.legend
echo S 0.1i c 0.16i - 0.25p 0.3i M 8 >> neis.legend
echo S 0.1i c 0.18i - 0.25p 0.3i M 9 >> neis.legend
echo V 0 1p >> neis.legend
echo D 0 1p >> neis.legend
echo N 1 >> neis.legend
echo ^> >> neis.legend
REM Put together a reasonable legend text, and add logo and user's name:
echo T USGS/NEIS most recent earthquakes for the last seven days.  The data were >> neis.legend
echo T obtained automatically from the USGS Earthquake Hazards Program page at >> neis.legend
echo T @_http://neic/usgs.gov @_.  Interested users may also receive email alerts >> neis.legend
echo T from the USGS. >> neis.legend
echo T This script can be called daily to update the latest information. >> neis.legend
echo G 0.4i >> neis.legend
REM Add USGS logo >> neis.legend
echo I USGS.ras 1i RT >> neis.legend
echo G -0.3i >> neis.legend
echo L 12 6 LB %me% >> neis.legend

REM OK, now we can actually run pslegend.  We center the legend below the map.
REM Trial and error shows that 1.7i is a good legend height:

pslegend -Dx4.5i/-0.4i/7i/1.7i/TC -Jx1i -R0/8/0/8 -O -F neis.legend -Glightyellow -S > legend.bat
if %master%==n echo OFF
call legend.bat >> example_22.ps
if %master%==n echo ON

REM Clean up after ourselves:

del .gmt*
del neis.*
del legend.bat
if %master%==y cd ..
