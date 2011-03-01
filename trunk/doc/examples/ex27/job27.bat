REM
REM		GMT EXAMPLE 27
REM		$Id: job27.bat,v 1.6 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Illustrates how to plot Mercator img grids
REM GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast
REM GMT supplement: img2grd (to read Sandwell/Smith img files)
REM DOS calls:	del, grep, $AWK
REM
echo GMT EXAMPLE 27
set master=y
if exist job27.bat set master=n
if %master%==y cd ex27

REM First extract a chunk of faa and retain short int precision to
REM save disk space.  Gravity is thus in 0.1 mGal increments.
REM Next get gradients.  The grid region is in Mercator x/y units

REM img2grd grav.15.2.img -R145/170/-50/-25 -M -C -T1 -Gtasman_grav.nc=ns
grdgradient tasman_grav.nc -Nt1 -A45 -Gtasman_grav_i.nc

REM Make a suitable cpt file for mGal

makecpt -T-120/120/10 -Z -Crainbow > grav.cpt

REM Since this is a Mercator grid we use a linear projection

grdimage tasman_grav.nc=ns/0.1 -Itasman_grav_i.nc -Jx0.25i -Cgrav.cpt -P -K -U"Example 27 in Cookbook" > ..\example_27.ps

REM Then use pscoast to plot land; get original -R from grid remark
REM and use Mercator projection with same scale as above on a spherical Earth

pscoast -R145/170/-50.0163575733/-24.9698584055 -Jm0.25i -Ba10f5WSne -O -K -Gblack --ELLIPSOID=Sphere -Cwhite -Dh+ --PLOT_DEGREE_FORMAT=dddF >> ..\example_27.ps

REM Put a color legend on top of the land mask justified with 147E,31S

echo 147E 31S 1 2.5 | psxy -R -J -O -K -Sr -D0.25i/0.05i -Gwhite -W1p --ELLIPSOID=Sphere --MEASURE_UNIT=inch >> ..\example_27.ps
psscale -D0.5i/6.3i/2i/0.15i -Cgrav.cpt -B50f10/:mGal: -I -O >> ..\example_27.ps

REM Clean up

del grav.cpt
del *_i.nc
del .gmt*
if %master%==y cd ..
