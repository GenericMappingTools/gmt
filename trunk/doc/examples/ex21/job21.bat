REM
REM             GMT EXAMPLE 21
REM
REM             $Id: job21.bat,v 1.13 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:    Plot a time-series
REM
REM GMT progs:	gmtset, gmtconvert, minmax, psbasemap, psxy 
REM DOS calls:  del
REM
echo GMT EXAMPLE 21
set master=y
if exist job21.bat set master=n
if %master%==y cd ex21


REM File has time stored as dd-Mon-yy so set input format to match it

gmtset INPUT_DATE_FORMAT dd-o-yy PLOT_DATE_FORMAT o ANNOT_FONT_SIZE_PRIMARY +10p
gmtset TIME_FORMAT_PRIMARY abbreviated CHAR_ENCODING ISOLatin1+

REM Create a suitable -R string

set R=-R1999-08-11T00:00:00/2006-11-01T00:00:00/0/300

REM Lay down the basemap:

psbasemap %R% -JX9i/6i -Glightgreen -K -U"Example 21 in Cookbook" -Bs1Y/WSen -Bpa3Of1o/50WSen:=$::."RedHat (RHAT) Stock Price Trend since IPO": > ..\example_21.ps

REM Plot main window with open price as red line over yellow envelope of low/highs

gmtset OUTPUT_DATE_FORMAT dd-o-yy
gmtconvert -F0,2 -f0T -Hi RHAT_price.csv > RHAT.env
gmtconvert -F0,3 -f0T -I -Hi RHAT_price.csv >> RHAT.env
psxy -R -J -Gyellow -O -K RHAT.env >> ..\example_21.ps
psxy -R -J RHAT_price.csv -H -Wthin,red -O -K >> ..\example_21.ps

REM Draw P Wessel's purchase price as line and label it.  Note we temporary switch
REM back to default yyyy-mm-dd format since that is what minmax gave us.

echo 05-May-00 0 > RHAT.pw
echo 05-May-00 300 >> RHAT.pw
psxy -R -J RHAT.pw -Wthinner,- -O -K >> ..\example_21.ps
echo 01-Jan-99 25 > RHAT.pw
echo 01-Jan-07 25 >> RHAT.pw
psxy -R -J RHAT.pw -Wthick,- -O -K >> ..\example_21.ps
gmtset INPUT_DATE_FORMAT yyyy-mm-dd
echo 1999-08-11T00:00:00 25 12 0 17 LB Wessel purchase price | pstext -R -J -O -K -D2i/0.05i -N >> ..\example_21.ps
gmtset INPUT_DATE_FORMAT dd-o-yy

REM Set smaller region for insert for trend since 2004

set R=-R2004T/2006-11-01T00:00:00/0/30

REM Lay down the basemap, using Finnish annotations and place the insert in the upper right:

gmtset TIME_LANGUAGE fi
psbasemap %R% -JX6i/3i -Bpa3Of3o/10:=$:ESw -Bs1Y/ -Glightblue -O -K -X3i -Y3i >> ..\example_21.ps
gmtset TIME_LANGUAGE us

REM Again, plot close price as red line over yellow envelope of low/highs

psxy -R -J -Gyellow -O -K RHAT.env >> ..\example_21.ps
psxy -R -J RHAT_price.csv -H -Wthin/red -O -K >> ..\example_21.ps

REM Draw P Wessel's purchase price as dashed line

psxy -R -J RHAT.pw -Wthick,- -O >> ..\example_21.ps

REM Clean up after ourselves:

del RHAT.*
del .gmt*
if %master%==y cd ..
