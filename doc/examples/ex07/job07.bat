REM		GMT EXAMPLE 07
REM
REM		$Id: job07.bat,v 1.8 2006-10-22 14:26:49 remko Exp $
REM
REM Purpose:	Make a basemap with earthquakes and isochrons etc
REM GMT progs:	pscoast, pstext, psxy
REM DOS calls:	del, echo, gawk
REM
echo GMT EXAMPLE 07
set master=y
if exist job07.bat set master=n
if %master%==y cd ex07
pscoast -R-50/0/-10/20 -JM9i -K -GP300/26 -Dl -Wthinnest -B10 -U"Example 7 in Cookbook" > example_07.ps
psxy -R -J -O -K -M fz.xy -Wthinner,- >> example_07.ps
gawk "{print $1-360.0, $2, $3*0.01}" quakes.xym | psxy -R -J -O -K -H1 -Sci -Gwhite -Wthinnest >> example_07.ps
psxy -R -J -O -K -M isochron.xy -Wthin >> example_07.ps
psxy -R -J -O -K -M ridge.xy -Wthicker >> example_07.ps
echo -14.5 15.2 > tmp
echo -2 15.2 >> tmp
echo -2 17.8 >> tmp
echo -14.5 17.8 >> tmp
psxy -R -J -O -K -Gwhite -Wthick -A tmp >> example_07.ps
echo -14.35 15.35 > tmp
echo -2.15 15.35 >> tmp
echo -2.15 17.65 >> tmp
echo -14.35 17.65 >> tmp
psxy -R -J -O -K -Gwhite -Wthinner -A tmp >> example_07.ps
echo -13.5 16.5 | psxy -R -J -O -K -Sc0.08i -Gwhite -Wthinner >> example_07.ps
echo -12.5 16.5 18 0 6 5 ISC Earthquakes | pstext -R -J -O -K >> example_07.ps
echo -43 -5 30 0 1 6 SOUTH > tmp
echo -43 -8 30 0 1 6 AMERICA >> tmp
echo -7 11 30 0 1 6 AFRICA >> tmp
pstext -R -J -O -Sthin -Gwhite tmp >> example_07.ps
del .gmt*
del tmp
if %master%==y cd ..
