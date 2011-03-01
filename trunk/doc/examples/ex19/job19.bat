REM		GMT EXAMPLE 19
REM
REM		$Id: job19.bat,v 1.18 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Illustrates various color pattern effects for maps
REM GMT progs:	gmtset, grdimage, grdmath, makecpt, pscoast, pstext, psimage
REM DOS calls:	echo, del
REM
echo GMT EXAMPLE 19
set master=y
if exist job19.bat set master=n
if %master%==y cd ex19
REM First make a worldmap with graded blue oceans and rainbow continents

gmtset COLOR_MODEL rgb
grdmath -Rd -I1 Y COSD 2 POW = lat.nc
grdmath -Rd -I1 X Y ABS 90 NEQ MUL = lon.nc
echo 0 white 1 blue > lat.cpt
makecpt -Crainbow -T-180/180/60 -Z > lon.cpt
grdimage lat.nc -Sl -JI0/6.5i -Clat.cpt -P -K -Y7.5i -B0 > ..\example_19.ps
pscoast -R -J -O -K -Dc -A5000 -Gc >> ..\example_19.ps
grdimage lon.nc -Sl -J -Clon.cpt -O -K >> ..\example_19.ps
pscoast -R -J -O -K -Q >> ..\example_19.ps
pscoast -R -J -O -K -Dc -A5000 -Wthinnest >> ..\example_19.ps
echo 0 20 32 0 1 CM 9TH INTERNATIONAL | pstext -R -J -O -K -Gred -Sthinner >> ..\example_19.ps
echo 0 -10 32 0 1 CM GMT CONFERENCE | pstext -R -J -O -K -Gred -Sthinner >> ..\example_19.ps
echo 0 -30 18 0 1 CM Honolulu, Hawaii, April 1, 2011 | pstext -R -J -O -K -Ggreen -Sthinnest >> ..\example_19.ps

REM Then show example of color patterns and placing a PostScript image

pscoast -R -J -O -K -Dc -A5000 -Gp100/86:FredByellow -Sp100/circuit.ras -B0 -Y-3.25i >> ..\example_19.ps
echo 0 30 32 0 1 CM SILLY USES OF | pstext -R -J -O -K -Glightgreen -Sthinner >> ..\example_19.ps
echo 0 -30 32 0 1 CM COLOR PATTERNS | pstext -R -J -O -K -Gmagenta -Sthinner >> ..\example_19.ps
psimage -C3.25i/1.625i/CM -W3i GMT_covertext.eps -O -K >> ..\example_19.ps

REM Finally repeat 1st plot but exchange the patterns

grdimage lon.nc -Sl -J -Clon.cpt -O -K -Y-3.25i -B0 -U"Example 19 in Cookbook" >> ..\example_19.ps
pscoast -R -J -O -K -Dc -A5000 -Gc >> ..\example_19.ps
grdimage lat.nc -Sl -J -Clat.cpt -O -K >> ..\example_19.ps
pscoast -R -J -O -K -Q >> ..\example_19.ps
pscoast -R -J -O -K -Dc -A5000 -Wthinnest >> ..\example_19.ps
echo 0 20 32 0 1 CM 9TH INTERNATIONAL | pstext -R -J -O -K -Gred -Sthinner >> ..\example_19.ps
echo 0 -10 32 0 1 CM GMT CONFERENCE | pstext -R -J -O -K -Gred -Sthinner >> ..\example_19.ps
echo 0 -30 18 0 1 CM Honolulu, Hawaii, April 1, 2011 | pstext -R -J -O -Ggreen -Sthinnest >> ..\example_19.ps

del l*.nc
del l*.cpt
del .gmt*
if %master%==y cd ..
