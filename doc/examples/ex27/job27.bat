REM
REM		GMT EXAMPLE 27
REM		$Id: job27.bat,v 1.7 2011-03-15 02:06:31 guru Exp $
REM
REM Purpose:	Illustrates how to plot Mercator img grids
REM GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast
REM GMT supplement: img2grd (to read Sandwell/Smith img files)
REM DOS calls:	del, grep, $AWK
REM
echo GMT EXAMPLE 27
set ps=..\example_27.ps

REM Gravity in tasman_grav.nc is in 0.1 mGal increments and the grid
REM is already in projected Mercator x/y units.
REM First get gradients.

grdgradient tasman_grav.nc -Nt1 -A45 -Gtasman_grav_i.nc

REM Make a suitable cpt file for mGal

makecpt -T-120/120/240 -Z -Crainbow > grav.cpt

REM Since this is a Mercator grid we use a linear projection

grdimage tasman_grav.nc=ns/0.1 -Itasman_grav_i.nc -Jx0.25i -Cgrav.cpt -P -K -U"Example 27 in Cookbook" > %ps%

REM Then use pscoast to plot land; get original -R from grid info
REM and use Mercator projection with same scale as above on a spherical Earth

pscoast -R145/170/-50.0163575733/-24.9698584055 -Jm0.25i -Ba10f5WSne -O -K -Gblack --PROJ_ELLIPSOID=Sphere -Cwhite -Dh+ --FORMAT_GEO_MAP=dddF >> %ps%

REM Put a color legend on top of the land mask justified with 147E,31S

echo 147E 31S 1i 2.5i | psxy -R -J -O -K -Sr -D0.25i/0.05i -Gwhite -W1p --PROJ_ELLIPSOID=Sphere >> %ps%
psscale -D0.5i/6.3i/2i/0.15i -Cgrav.cpt -B50f10/:mGal: -I -O >> %ps%

REM Clean up

del grav.cpt
del *_i.nc
del .gmt*
