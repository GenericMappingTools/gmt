REM		GMT EXAMPLE 19
REM
REM		$Id: job19.bat,v 1.6 2004-09-29 01:29:02 pwessel Exp $
REM
REM Purpose:	Illustrates various color pattern effects for maps
REM GMT progs:	gmtset, grdimage, grdmath, makecpt, pscoast, pstext
REM DOS calls:	echo, del
REM
echo GMT EXAMPLE 19
set master=y
if exist job19.bat set master=n
if %master%==y cd ex19
REM First make a worldmap with graded blue oceans and rainbow continents

gmtset COLOR_MODEL rgb
grdmath -R-180/180/-90/90 -I1 -F Y COSD 2 POW = lat.grd
grdmath -R-180/180/-90/90 -I1 -F X = lon.grd
echo 0 255 255 255 1 0 0 255 > lat.cpt
makecpt -Crainbow -T-180/180/60 -Z > lon.cpt
grdimage lat.grd -JI0/6.5i -Clat.cpt -P -K -Y7.5i -B0 > example_19.ps
pscoast -R -J -O -K -Dc -A5000 -Gc >> example_19.ps
grdimage lon.grd -J -Clon.cpt -O -K >> example_19.ps
pscoast -R -J -O -K -Q >> example_19.ps
pscoast -R -J -O -K -Dc -A5000 -W0.25p >> example_19.ps
echo 0 20 32 0 1 CM FIRST INTERNATIONAL | pstext -R -J -O -K -Gred -S0.5p >> example_19.ps
echo 0 -10 32 0 1 CM GMT CONFERENCE | pstext -R -J -O -K -Gred -S0.5p >> example_19.ps
echo 0 -30 18 0 1 CM Honolulu, Hawaii, April 1, 2000 | pstext -R -J -O -K -Ggreen -S0.25p >> example_19.ps

REM Then show example of color patterns

pscoast -R -J -O -K -Dc -A5000 -Gp100/86:FredByellow -Sp100/7:FredBblack -B0 -Y-3.25i >> example_19.ps
echo 0 15 32 0 1 CM SILLY USES OF | pstext -R -J -O -K -Glightgreen -S0.5p >> example_19.ps
echo 0 -15 32 0 1 CM GMT COLOR PATTERNS | pstext -R -J -O -K -Gmagenta -S0.5p >> example_19.ps

REM Finally repeat 1st plot but exchange the patterns

grdimage lon.grd -J -Clon.cpt -O -K -Y-3.25i -B0 -U"Example 19 in Cookbook" >> example_19.ps
pscoast -R -J -O -K -Dc -A5000 -Gc >> example_19.ps
grdimage lat.grd -J -Clat.cpt -O -K >> example_19.ps
pscoast -R -J -O -K -Q >> example_19.ps
pscoast -R -J -O -K -Dc -A5000 -W0.25p >> example_19.ps
echo 0 20 32 0 1 CM 3RD INTERNATIONAL | pstext -R -J -O -K -Gred -S0.5p >> example_19.ps
echo 0 -10 32 0 1 CM GMT CONFERENCE | pstext -R -J -O -K -Gred -S0.5p >> example_19.ps
echo 0 -30 18 0 1 CM Honolulu, Hawaii, April 1, 2005 | pstext -R -J -O -Ggreen -S0.25p >> example_19.ps

del l*.grd
del l*.cpt
del .gmt*
if %master%==y cd ..
