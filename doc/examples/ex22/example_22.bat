REM
REM             GMT EXAMPLE 22
REM
REM             $Id$
REM
REM Purpose:    Automatic map of last month of world-wide seismicity
REM
REM GMT progs:  gmtset, pscoast, psxy, pslegend
REM DOS calls:  del, sed, awk, wget|curl
REM
echo GMT EXAMPLE 22
set ps=example_22.ps

gmt set FONT_ANNOT_PRIMARY 10p FONT_TITLE 18p FORMAT_GEO_MAP ddd:mm:ssF

REM Get the data (-s silently) from USGS using the curl)
REM Hardwired here to the month of October, 2017
REM SITE="https://earthquake.usgs.gov/fdsnws/event/1/query.csv"
REM TIME="starttime=2017-09-01%2000:00:00&endtime=2017-10-01%2000:00:00"
REM MAG="minmagnitude=3"
REM ORDER="orderby=magnitude"
REM URL="${SITE}?${TIME}&${MAG}&${ORDER}"
REM curl -s $URL > usgs_quakes_22.txt

REM Count the number of events (to be used in title later. one less due to header)

gmt which @usgs_quakes_22.txt -Gl
REM n=`gmt info usgs_quakes_22.txt -h1 -Fi -o2`
REM n=`expr $n - 1`
set n=77

REM Pull out the first and last timestamp to use in legend title

REM first=`sed -n 2p usgs_quakes_22.txt | awk -F, '{printf "%s\n", substr($1,1,10)}'`
REM last=`sed -n '$p' usgs_quakes_22.txt | awk -F, '{printf "%s\n", substr($1,1,10)}'`
set first=2017-09-08
set last=2017-09-29

REM Assign a string that contains the current user @ the current computer node.
REM Note that two @@ is needed to print a single @ in gmt pstext:

REM set me = "$user@@`hostname`"
set me=GMT guru @@ GMTbox

REM Create standard seismicity color table

gmt makecpt -Cred,green,blue -T0,100,300,10000 -N > neis.cpt

REM Start plotting. First lay down map, then plot quakes with size = magintude * 0.015":

gmt pscoast -Rg -JK180/9i -B45g30 -B+t"World-wide earthquake activity" -Gburlywood -Slightblue -Dc -A1000 -K -Y2.75i > %ps%
gmt psxy -R -J -O -K -Cneis.cpt -Sci -Wfaint -hi1 -i2,1,3,4+s0.015 usgs_quakes_22.txt >> %ps%

REM Create legend input file for NEIS quake plot

echo H 16 1 %n% events during %first% to %last% > neis.legend
echo D 0 1p >> neis.legend
echo N 3 >> neis.legend
echo V 0 1p >> neis.legend
echo S 0.1i c 0.1i red 0.25p 0.2i Shallow depth (0-100 km) >> neis.legend
echo S 0.1i c 0.1i green 0.25p 0.2i Intermediate depth (100-300 km) >> neis.legend
echo S 0.1i c 0.1i blue 0.25p 0.2i Very deep (> 300 km) >> neis.legend
echo D 0 1p >> neis.legend
echo V 0 1p >> neis.legend
echo N 7 >> neis.legend
echo V 0 1p >> neis.legend
echo S 0.1i c 0.06i - 0.25p 0.3i M 3 >> neis.legend
echo S 0.1i c 0.08i - 0.25p 0.3i M 4 >> neis.legend
echo S 0.1i c 0.10i - 0.25p 0.3i M 5 >> neis.legend
echo S 0.1i c 0.12i - 0.25p 0.3i M 6 >> neis.legend
echo S 0.1i c 0.14i - 0.25p 0.3i M 7 >> neis.legend
echo S 0.1i c 0.16i - 0.25p 0.3i M 8 >> neis.legend
echo S 0.1i c 0.18i - 0.25p 0.3i M 9 >> neis.legend
echo D 0 1p >> neis.legend
echo V 0 1p >> neis.legend
echo N 1 >> neis.legend

REM Put together a reasonable legend text, and add logo and user's name:
echo P >> neis.legend
echo T USGS/NEIS most recent earthquakes for the last month.  The data were >> neis.legend
echo T obtained automatically from the USGS Earthquake Hazards Program page at >> neis.legend
echo T @_http://neic/usgs.gov @_.  Interested users may also receive email alerts >> neis.legend
echo T from the USGS. >> neis.legend
echo T This script could be called monthly to update the latest information. >> neis.legend
echo G 0.4i >> neis.legend
echo I @USGS.ras 1i RT >> neis.legend
echo G -0.3i >> neis.legend
echo L 12 6 LB %me% >> neis.legend

REM OK, now we can actually run gmt pslegend.  We center the legend below the map.
REM Trial and error shows that 1.7i is a good legend height:

gmt pslegend -DjCB+o0/0.4i+w7i/1.7i+jTC -R -J -O -F+p+glightyellow neis.legend >> %ps%

REM Clean up after ourselves:

del neis.*
del .gmt*
del gmt.conf
