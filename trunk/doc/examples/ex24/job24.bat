REM
REM             GMT EXAMPLE 24
REM
REM             $Id$
REM
REM Purpose:    Extract subsets of data based on geospatial criteria
REM
REM GMT progs:  gmtselect, pscoast, psxy
REM DOS calls:  del
REM
echo GMT EXAMPLE 24
set ps=..\example_24.ps

REM Highlight oceanic earthquakes within 3000 km of Hobart and > 1000 km from dateline
echo 147:13 -42:48 3000 Hobart > point.d
REM Our proxy for the dateline
echo 62 | gawk "{printf \"%%c\n\", $1}" > dateline.d 
echo 180 0 >> dateline.d
echo 180 -90 >> dateline.d
set R=-R100/200/-60/0
pscoast %R% -JM9i -K -Gtan -Sdarkblue -Wthin,white -Dl -A500 -Ba20f10g10WeSn -U"Example 24 in Cookbook" > %ps%
psxy -R -J -O -K oz_quakes.d -Sc0.05i -Gred >> %ps%
gmtselect oz_quakes.d -L1000k/dateline.d -Nk/s -C3000k/point.d -fg -R -Il | psxy -R -JM -O -K -Sc0.05i -Ggreen >> %ps%
echo {print $1, $2, 0, 2*$3, 2*$3} > awk.txt
gawk -f awk.txt point.d | psxy -R -J -O -K -SE -Wfat,white >> %ps%
echo {print $1, $2, $4} > awk.txt
gawk -f awk.txt point.d | pstext -R -J -O -K -D0.1i/-0.1i -F+f14p,Helvetica-Bold,white+jLT >> %ps%
psxy -R -J -O -K point.d -Wfat,white -S+0.2i >> %ps%
psxy -R -J -O dateline.d -Wfat,white -A >> %ps%
del point.d
del dateline.d
del awk.txt
del .gmt*
