REM
REM		GMT EXAMPLE 21
REM
REM		$Id: job21.bat,v 1.2 2004-04-25 09:04:36 pwessel Exp $
REM
REM Purpose:	Plot a time-series
REM GMT progs:	gmtset, gmtconvert, minmax, psbasemap, psxy
REM DOS calls:	echo
REM
echo GMT EXAMPLE 21
set master=y
if exist job21.bat set master=n
if %master%==y cd ex21

REM File has time stored as dd-Mon-yy so set input format to match it

gmtset INPUT_DATE_FORMAT dd-o-yy PLOT_DATE_FORMAT o ANNOT_FONT_SIZE +10p
gmtset TIME_FORMAT_PRIMARY abbreviated CHAR_ENCODING ISOLatin1+

REM Pull out a suitable region string in yyy-mm-dd format

set w=1998-08-11T
set e=2004-03-12T
set s=0
set n=300
set R="-R%w%/%e%/%s%/%n%"

REM Lay down the basemap:

psbasemap %R% -JX9iT/6i -Glightgreen -K -U"Example 21 in Cookbook" -Bpa3Of1o/50WSen:^$::."RedHat (RHAT) Stock Price Trend since IPO": -Bs1Y/0WSen > example_21.ps

REM Plot main window with close price as red line over yellow envelope of low/highs

gmtset OUTPUT_DATE_FORMAT dd-o-yy
gmtconvert -F0,2 -f0T -Hi RHAT_price.csv > RHAT.env
gmtconvert -F0,3 -f0T -I -Hi RHAT_price.csv >> RHAT.env
psxy -R -J -Gyellow -O -K RHAT.env >> example_21.ps
psxy -R -J RHAT_price.csv -H -Wthin,red -O -K >> example_21.ps

REM Draw P Wessel's purchase price as line and label it.  Note we temporary switch
REM back to default yyyy-mm-dd format since that is what minmax gave us.

echo "05-May-00	0" > RHAT.pw
echo "05-May-00	300" >> RHAT.pw
psxy -R -J RHAT.pw -Wthinner,- -O -K >> example_21.ps
echo "01-Jan-99	25" > RHAT.pw
echo "01-Jan-05	25" >> RHAT.pw
psxy -R -J RHAT.pw -Wthick,- -O -K >> example_21.ps
gmtset INPUT_DATE_FORMAT yyyy-mm-dd
echo "%e% 25 12 0 17 RB Wessel purchase price" | pstext -R -J -O -K -D-0.1i/0.05i -N >> example_21.ps
gmtset INPUT_DATE_FORMAT dd-o-yy

REM Get smaller region for insert for trend since 2003

R="-R2003T/%e%/%s%/30"

REM Lay down the basemap, using Finnish annotations and place the insert in the upper right:

gmtset TIME_LANGUAGE fi
psbasemap %R% -JX6iT/3i -Bpa1Of3o/10:^$:ESw -Bs1Y/ -Glightblue -O -K -X3i -Y3i >> example_21.ps
gmtset TIME_LANGUAGE us

REM Again, plot close price as red line over yellow envelope of low/highs

psxy -R -J -Gyellow -O -K RHAT.env >> example_21.ps
psxy -R -J RHAT_price.csv -H -Wthin/red -O -K >> example_21.ps

REM Draw P Wessel's purchase price as dashed line

psxy -R -J RHAT.pw -Wthick,- -O >> example_21.ps

REM Clean up after ourselves:

del RHAT.*
del .gmt*

if %master%==y cd ..
