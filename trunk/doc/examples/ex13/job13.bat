REM		GMT EXAMPLE 13
REM
REM		$Id: job13.bat,v 1.9 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Illustrate vectors and contouring
REM GMT progs:	grdmath, grdcontour, grdvector, pstext
REM DOS calls:	echo, del
REM
echo GMT EXAMPLE 13
set master=y
if exist job13.bat set master=n
if %master%==y cd ex13
grdmath -R-2/2/-2/2 -I0.1 X Y R2 NEG EXP X MUL = z.nc
grdmath z.nc DDX = dzdx.nc
grdmath z.nc DDY = dzdy.nc
grdcontour dzdx.nc -JX3i -B1/1WSne -C0.1 -A0.5 -K -P -Gd2i -S4 -T0.1i/0.03i -U"Example 13 in Cookbook" > ..\example_13.ps
grdcontour dzdy.nc -J -B1/1WSne -C0.05 -A0.2 -O -K -Gd2i -S4 -T0.1i/0.03i -X3.45i >> ..\example_13.ps
grdcontour z.nc -J -B1/1WSne -C0.05 -A0.1 -O -K -Gd2i -S4 -T0.1i/0.03i -X-3.45i -Y3.45i >> ..\example_13.ps
grdcontour z.nc -J -B1/1WSne -C0.05 -O -K -Gd2i -S4 -X3.45i >> ..\example_13.ps
grdvector dzdx.nc dzdy.nc -I0.2 -J -O -K -Q0.03i/0.1i/0.09in0.25i -G0 -S5i >> ..\example_13.ps
echo 3.2 3.6 40 0 6 2 z(x,y) = x * exp(-x@+2@+-y@+2@+) | pstext -R0/6/0/4.5 -Jx1i -O -X-3.45i >> ..\example_13.ps
del *.nc
del .gmt*
if %master%==y cd ..
