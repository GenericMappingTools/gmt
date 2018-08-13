REM
REM             GMT EXAMPLE 24
REM
REM
REM Purpose:    Extract subsets of data based on geospatial criteria
REM
REM GMT progs:  gmtselect, pscoast, psxy
REM DOS calls:  del
REM
echo GMT EXAMPLE 24
set ps=example_24.ps

REM Highlight oceanic earthquakes within 3000 km of Hobart and > 1000 km from dateline
echo 147:13 -42:48 6000 > point.txt
REM Our proxy for the dateline
echo 62 | gawk "{printf \"%%c\n\", $1}" > dateline.txt 
echo 180 0 >> dateline.txt
echo 180 -90 >> dateline.txt
set R=-R100/200/-60/0
gmt pscoast %R% -JM9i -K -Gtan -Sdarkblue -Wthin,white -Dl -A500 -Ba20f10g10 -BWeSn > %ps%
gmt psxy -R -J -O -K @oz_quakes_24.txt -Sc0.05i -Gred >> %ps%
gmt select @oz_quakes_24.txt -Ldateline.txt+d1000k -Nk/s -Cpoint.txt+d3000k -fg -R -Il | gmt psxy -R -JM -O -K -Sc0.05i -Ggreen >> %ps%
gmt psxy point.txt -R -J -O -K -SE- -Wfat,white >> %ps%
gmt pstext point.txt -R -J -O -K -D0.1i/-0.1i -F+f14p,Helvetica-Bold,white+jLT+tHobart >> %ps%
gmt psxy -R -J -O -K point.txt -Wfat,white -S+0.2i >> %ps%
gmt psxy -R -J -O dateline.txt -Wfat,white -A >> %ps%
del point.txt
del dateline.txt
del awk.txt
del .gmt*
