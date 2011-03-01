REM             GMT EXAMPLE 29
REM             $Id: job29.bat,v 1.6 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Illustrates spherical surface gridding with Green's function of splines
REM GMT progs:	makecpt, grdcontour, grdgradient, grdimage, grdmath greenspline, psscale, pstext
REM DOS calls:	del, echo
REM

echo GMT EXAMPLE 29
set master=y
if exist job29.bat set master=n
if %master%==y cd ex29

REM This example uses 370 radio occultation data for Mars to grid the topography.
REM Data and information from Smith, D. E., and M. T. Zuber (1996), The shape of
REM Mars and the topographic signature of the hemispheric dichotomy, Science, 271, 184–187.

REM Make Mars ellipsoid given their three best-fitting axes:
set a=3399.472
set b=3394.329
set c=3376.502
grdmath -Rg -I4 -F X COSD %a% DIV DUP MUL X SIND %b% DIV DUP MUL ADD Y COSD DUP MUL MUL Y SIND %c% DIV DUP MUL ADD SQRT INV = ellipsoid.nc

REM  Do both Parker and Wessel/Becker solutions (tension = 0.9975)
greenspline -Rellipsoid.nc mars370.in -D4 -Sp -Gmars.nc
greenspline -Rellipsoid.nc mars370.in -D4 -SQ0.9975/5001 -Gmars2.nc
REM Scale to km and remove ellipsoid
grdmath mars.nc 1000 DIV ellipsoid.nc SUB = mars.nc
grdmath mars2.nc 1000 DIV ellipsoid.nc SUB = mars2.nc
makecpt -Crainbow -T-7/15/1 -Z > mars.cpt
grdgradient mars2.nc -M -Ne0.75 -A45 -Gmars2_i.nc
grdimage mars2.nc -Imars2_i.nc -Cmars.cpt -B30g30Wsne -JH0/6i -P -K -Ei -U"Example 29 in Cookbook" --ANNOT_FONT_SIZE_PRIMARY=12 > ..\example_29.ps
grdcontour mars2.nc -J -O -K -C1 -A5 -Glz+/z- >> ..\example_29.ps
psxy -Rg -J -O -K -Sc0.045i -Gblack mars370.in  >> ..\example_29.ps
echo 0 90 14 0 1 LB b) | pstext -R -J -O -K -N -D-3i/-0.2i >> ..\example_29.ps
grdgradient mars.nc -M -Ne0.75 -A45 -Gmars_i.nc
grdimage mars.nc -Imars_i.nc -Cmars.cpt -B30g30Wsne -J -O -K -Ei -Y3.6i --ANNOT_FONT_SIZE_PRIMARY=12 >> ..\example_29.ps
grdcontour mars.nc -J -O -K -C1 -A5 -Glz+/z- >> ..\example_29.ps
psxy -Rg -J -O -K -Sc0.045i -Gblack mars370.in  >> ..\example_29.ps
psscale -Cmars.cpt -O -K -D3i/-0.1i/5i/0.1ih -I --ANNOT_FONT_SIZE_PRIMARY=12 -B2f1/:km: >> ..\example_29.ps
echo 0 90 14 0 1 LB a) | pstext -R -J -O -N -D-3i/-0.2i >> ..\example_29.ps
REM Clean up
del *.nc
del mars.cpt
del .gmt*
if %master%==y cd ..
