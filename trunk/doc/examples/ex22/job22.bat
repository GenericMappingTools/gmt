REM
REM             GMT EXAMPLE 22
REM
REM             $Id$
REM
REM Purpose:    Automatic map of last 7 days of world-wide seismicity
REM
REM GMT progs:  gmtset, pscoast, psxy, pslegend
REM DOS calls:  del, sed, awk, wget|curl
REM
echo GMT EXAMPLE 22
set ps=..\example_22.ps

gmtset FONT_ANNOT_PRIMARY 10p FONT_TITLE 18p FORMAT_GEO_MAP ddd:mm:ssF

REM Get the data (-q quietly) from USGS using the wget (comment out in case
REM your system does not have wget or curl)

REM wget http://neic.usgs.gov/neis/gis/bulletin.asc -q -O neic_quakes.d
REM curl http://neic.usgs.gov/neis/gis/bulletin.asc -s > neic_quakes.d

REM Count the number of events (to be used in title later. one less due to header)

REM n=`cat neic_quakes.d | wc -l`
REM n=`expr $n - 1`
set n=77

REM Pull out the first and last timestamp to use in legend title

REM first=`sed -n 2p neic_quakes.d | awk -F, '{printf "%s %s\n", $1, $2}'`
REM last=`sed -n '$p' neic_quakes.d | awk -F, '{printf "%s %s\n", $1, $2}'`
set first=04/04/19 00:04:33
set last=04/04/25 11:11:33

REM Assign a string that contains the current user @ the current computer node.
REM Note that two @@ is needed to print a single @ in pstext:

REM set me = "$user@@`hostname`"
set me=GMT guru @@ GMTbox

REM Create standard seismicity color table

echo 0	red	100	red > neis.cpt
echo 100	green	300	green >> neis.cpt
echo 300	blue	10000	blue >> neis.cpt

REM Start plotting. First lay down map, then plot quakes with size = magintude/50":

pscoast -Rg -JK180/9i -B45g30:."World-wide earthquake activity": -Gbrown -Slightblue -Dc -A1000 -K -U/-0.75i/-2.5i/"Example 22 in Cookbook" -Y2.75i > %ps%
gawk -F, "{ print $4, $3, $6, $5*0.02}" neic_quakes.d | psxy -R -JK -O -K -Cneis.cpt -Sci -Wthin -h >> %ps%

REM Create legend input file for NEIS quake plot

echo H 16 1 %n% events during %first% to %last% > neis.legend
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

REM Put together a reasonable legend text, and add logo and user's name:
echo P >> neis.legend
echo T USGS/NEIS most recent earthquakes for the last seven days.  The data were >> neis.legend
echo T obtained automatically from the USGS Earthquake Hazards Program page at >> neis.legend
echo T @_http://neic/usgs.gov @_.  Interested users may also receive email alerts >> neis.legend
echo T from the USGS. >> neis.legend
echo T This script can be called daily to update the latest information. >> neis.legend
echo G 0.4i >> neis.legend
echo I USGS.ras 1i RT >> neis.legend
echo G -0.3i >> neis.legend
echo L 12 6 LB %me% >> neis.legend

REM OK, now we can actually run pslegend.  We center the legend below the map.
REM Trial and error shows that 1.7i is a good legend height:

pslegend -Dx4.5i/-0.4i/7i/1.7i/TC -O -F neis.legend -Glightyellow >> %ps%

REM Clean up after ourselves:

del neis.*
del .gmt*
