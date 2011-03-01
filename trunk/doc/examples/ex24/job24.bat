REM
REM             GMT EXAMPLE 24
REM
REM             $Id: job24.bat,v 1.14 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:    Extract subsets of data based on geospatial criteria
REM
REM GMT progs:  gmtselect, pscoast, psxy
REM DOS calls:  del
REM
echo GMT EXAMPLE 24
set master=y
if exist job24.bat set master=n
if %master%==y cd ex24

REM Highlight oceanic earthquakes within 3000 km of Hobart and > 1000 km from dateline
echo 147:13 -42:48 3000 Hobart > point.d
REM Our proxy for the dateline
echo # > dateline.d
echo 180 0 >> dateline.d
echo 180 -90 >> dateline.d
set R=-R100/200/-60/0
pscoast %R% -JM9i -K -Gtan -Sdarkblue -Wthin,white -Dl -A500 -Ba20f10g10WeSn -U"Example 24 in Cookbook" > ..\example_24.ps
psxy -R -J -O -K oz_quakes.d -Sc0.05i -Gred >> ..\example_24.ps
gmtselect oz_quakes.d -L1000/dateline.d -m# -Nk/s -C3000/point.d -fg -R -Il | psxy -R -JM -O -K -Sc0.05i -Ggreen >> ..\example_24.ps
echo {print $1, $2, 0, 2*$3, 2*$3} > awk.txt
gawk -f awk.txt point.d | psxy -R -J -O -K -SE -Wfat,white >> ..\example_24.ps
echo {print $1, $2, 14, 0, 1, "LT", $4} > awk.txt
gawk -f awk.txt point.d | pstext -R -J -O -K -Gwhite -D0.1i/-0.1i >> ..\example_24.ps
psxy -R -J -O -K point.d -Wfat,white -S+0.2i >> ..\example_24.ps
psxy -R -J -O -m dateline.d -Wfat,white -A >> ..\example_24.ps
del point.d
del dateline.d
del awk.txt
del .gmt*
if %master%==y cd ..

