REM
REM             GMT EXAMPLE 21
REM
REM
REM Purpose:    Plot a time-series
REM
REM GMT progs:	gmtset, gmtconvert, gmtinfo, psbasemap, psxy 
REM DOS calls:  del
REM
echo GMT EXAMPLE 21
set ps=example_21.ps


REM File has time stored as dd-Mon-yy so set input format to match it

gmt set FORMAT_DATE_IN dd-o-yy FORMAT_DATE_MAP o FONT_ANNOT_PRIMARY +10p
gmt set FORMAT_TIME_PRIMARY_MAP abbreviated PS_CHAR_ENCODING ISOLatin1+

REM Create a suitable -R string

set R=-R1999-08-04T12:00:00/2008-01-29T12:00:00/0/300

REM Lay down the basemap:

gmt psbasemap %R% -JX9i/6i -K -Bsx1Y -Bpxa3Of1o -Bpy50+p"$ " -BWSen+t"RedHat (RHT) Stock Price Trend since IPO"+glightgreen > %ps%

REM Plot main window with open price as red line over yellow envelope of low/highs

gmt set FORMAT_DATE_OUT dd-o-yy
gmt convert -o0,2 -f0T @RHAT_price.csv > RHAT.env
gmt convert -o0,3 -f0T -I -T @RHAT_price.csv >> RHAT.env
gmt psxy -R -J -Gyellow -O -K RHAT.env >> %ps%
gmt psxy -R -J @RHAT_price.csv -Wthin,red -O -K >> %ps%

REM Draw P Wessel's purchase price as line and label it.  Note we temporary switch
REM back to default yyyy-mm-dd format since that is what gmt info gave us.

echo 05-May-00 0 > RHAT.pw
echo 05-May-00 300 >> RHAT.pw
gmt psxy -R -J RHAT.pw -Wthinner,- -O -K >> %ps%
echo 01-Jan-99 25 > RHAT.pw
echo 01-Jan-02 25 >> RHAT.pw
gmt psxy -R -J RHAT.pw -Wthick,- -O -K >> %ps%
gmt set FORMAT_DATE_IN yyyy-mm-dd
echo 1999-08-11T00:00:00 25 PW buy | gmt pstext -R -J -O -K -D1.5i/0.05i -F+f12p,Bookman-Demi+jLB -N >> %ps%
gmt set FORMAT_DATE_IN dd-o-yy

REM Draw P Wessel's sales price as line and label it.

echo 25-Jun-07 0 > RHAT.pw
echo 25-Jun-07 300 >> RHAT.pw
gmt psxy -R -J RHAT.pw -Wthinner,- -O -K >> %ps%
echo 01-Aug-06 23.8852 > RHAT.pw
echo 01-Jan-08 23.8852 >> RHAT.pw
gmt psxy -R -J RHAT.pw -Wthick,- -O -K >> %ps%
gmt set FORMAT_DATE_IN yyyy-mm-dd
rem echo "2007-06-25T00:00:00 23.8852 PW sell" | gmt pstext -R -J -O -K -Dj0.8i/0.05i -N -F+f12p,Bookman-Demi+jRB >> %ps%
echo 2008-01-29T12:00:00 23.8852 PW sell | gmt pstext -R -J -O -K -Dj0.8i/0.05i -N -F+f12p,Bookman-Demi+jRB >> %ps%
gmt set FORMAT_DATE_IN dd-o-yy

REM Set smaller region for inset for trend since 2004

set R=-R2004T/2007-12-31T00:00:00/0/30

REM Lay down the basemap, using Finnish annotations and place the inset in the upper right

gmt psbasemap --TIME_LANGUAGE=fi %R% -JX6i/3i -Bpxa3Of3o -Bpy10+p"$ " -BESw+glightblue -Bsx1Y -O -K -X3i -Y3i >> %ps%

REM Again, plot close price as red line over yellow envelope of low/highs

gmt psxy -R -J -Gyellow -O -K RHAT.env >> %ps%
gmt psxy -R -J @RHAT_price.csv -Wthin,red -O -K >> %ps%

REM Draw P Wessel's purchase price as dashed line

gmt psxy -R -J RHAT.pw -Wthick,- -O -K >> %ps%

REM Mark sales date

echo 25-Jun-07 0 > RHAT.pw
echo 25-Jun-07 300 >> RHAT.pw
gmt psxy -R -J RHAT.pw -Wthinner,- -O >> %ps%

REM Clean up after ourselves:

del RHAT.*
del .gmt*
del gmt.conf
