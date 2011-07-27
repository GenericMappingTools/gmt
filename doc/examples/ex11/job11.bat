REM		$Id$
REM		GMT EXAMPLE 11
REM
REM Purpose:	Create a 3-D RGB Cube
REM GMT progs:	gmtset, grdimage, grdmath, pstext, psxy
REM DOS calls:	echo, del

REM Use psxy to plot "cut-along-the-dotted" lines.
echo GMT EXAMPLE 11
set ps=..\example_11.ps

gmtset MAP_TICK_LENGTH_PRIMARY 0

psxy cut-here.dat -Wthinnest,. -R-51/306/0/1071 -JX3.5i/10.5i -X2.5i -Y0.5i -P -U/-2.0i/-0.2i/"Example 11 in Cookbook" -K > %ps%

REM First, create grids of ascending X and Y and constant 0.
REM These are to be used to represent R, G and B values of the darker 3 faces of the cube.

grdmath -I1 -R0/255/0/255 X = x.nc
grdmath -I1 -R Y = y.nc
grdmath -I1 -R 0 = c.nc

gmtset FONT_ANNOT_PRIMARY 12p,Helvetica-Bold

grdimage x.nc y.nc c.nc -JX2.5i/-2.5i -R -K -O -X0.5i >> %ps%
psxy rays.dat -J -R -K -O -Bwesn >> %ps%
echo 128 128 60\217 | pstext -J -R -F+fwhite+a-45 -K -O >> %ps%
echo 102  26 0.4 | pstext -J -R -F+fwhite+a-90 -K -O >> %ps%
echo 204  26 0.8 | pstext -J -R -F+fwhite+a-90 -K -O >> %ps%
echo 10 140 G | pstext -J -R -F+f16p,white+a180 -K -O >> %ps%
echo 0 0 0 128 | psxy -N -Svs -Gwhite -J -R -K -O >> %ps%

grdimage x.nc c.nc y.nc -JX2.5i/2.5i -R -K -O -Y2.5i >> %ps%
psxy rays.dat -J -R -K -O -Bwesn >> %ps%
echo 128 128 300\217 | pstext -J -R -F+fwhite+a45 -K -O >> %ps%
echo 26 102 0.4 | pstext -J -R -F+fwhite -K -O >> %ps%
echo 26 204 0.8 | pstext -J -R -F+fwhite -K -O >> %ps%
echo 140 10 R | pstext -J -R -F+f16p,white+a-90 -K -O >> %ps%
echo 100 100 V | pstext -J -R -F+f16p,white+a-45 -K -O >> %ps%
echo 0 0 128 0 | psxy -N -Svs -Gwhite -J -R -K -O >> %ps%
echo 0 0 90 90 | psxy -N -Svs -Gwhite -J -R -K -O >> %ps%

grdimage c.nc x.nc y.nc -JX-2.5i/2.5i -R -K -O -X-2.5i >> %ps%
psxy rays.dat -J -R -K -O -Bwesn >> %ps%
echo 128 128 180\217 | pstext -J -R -F+fwhite+a135 -K -O >> %ps%
echo 102 26 0.4 | pstext -J -R -F+fwhite+a90 -K -O >> %ps%
echo 204 26 0.8 | pstext -J -R -F+fwhite+a90 -K -O >> %ps%
echo 10 140 B | pstext --FONT=white -J -R -F+f16p,white -K -O >> %ps%
echo 0 0 0 128 | psxy -N -Svs -Gwhite -J -R -K -O >> %ps%
echo 0 0 128 0 | psxy -N -Svs -Gwhite -J -R -K -O >> %ps%

REM Second, create grids of descending X and Y and constant 255.
REM These are to be used to represent R, G and B values of the lighter 3 faces of the cube.

grdmath -I1 -R 255 X SUB = x.nc
grdmath -I1 -R 255 Y SUB = y.nc
grdmath -I1 -R 255       = c.nc

grdimage x.nc y.nc c.nc -JX-2.5i/-2.5i -R -K -O -X2.5i -Y2.5i >> %ps%
psxy rays.dat -J -R -K -O -Bwesn >> %ps%
echo 128 128 240\217 | pstext -J -R -F+a225 -K -O >> %ps%
echo 102 26 0.4 | pstext -J -R -F+a270 -K -O >> %ps%
echo 204 26 0.8 | pstext -J -R -F+a270 -K -O >> %ps%

grdimage c.nc y.nc x.nc -JX2.5i/-2.5i -R -K -O -X2.5i >> %ps%
psxy rays.dat -J -R -K -O -Bwesn >> %ps%
echo 128 128 0\217 | pstext -J -R -F+a-45 -K -O >> %ps%
echo 26 102 0.4 | pstext -J -R -K -O >> %ps%
echo 26 204 0.8 | pstext -J -R -K -O >> %ps%
echo 100 100 S | pstext -J -R -F+f16p+a45 -K -O >> %ps%
echo 204 66 H | pstext -J -R -F+f16p+a90 -K -O >> %ps%
echo 0 0 90 90 | psxy -N -Svs -Gblack -J -R -K -O >> %ps%
echo 204 204 204 76 | psxy -N -Svs -Gblack -J -R -K -O >> %ps%

grdimage x.nc c.nc y.nc -JX-2.5i/2.5i -R -K -O -X-2.5i -Y2.5i >> %ps%
psxy rays.dat -J -R -K -O -Bwesn >> %ps%
echo 128 128 120\217 | pstext -J -R -F+a135 -K -O >> %ps%
echo 26 102 0.4 | pstext -J -R -F+a180 -K -O >> %ps%
echo 26 204 0.8 | pstext -J -R -F+a180 -K -O >> %ps%
echo 200 200 GMT 4 | pstext -J -F+f16p+a225 -R -O >> %ps%

del *.nc
del .gmt*
