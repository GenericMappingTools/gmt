REM		$Id$
REM		GMT EXAMPLE 11
REM
REM Purpose:	Create a 3-D RGB Cube
REM GMT progs:	gmtset, grdimage, grdmath, pstext, psxy
REM DOS calls:	echo, del

REM Use psxy to plot "cut-along-the-dotted" lines.
echo GMT EXAMPLE 11
set ps=example_11.ps

gmt gmtset MAP_TICK_LENGTH_PRIMARY 0

gmt psxy cut-here.dat -Wthinnest,. -R-51/306/0/1071 -JX3.5i/10.5i -X2.5i -Y0.5i -P -K > %ps%

REM First, create grids of ascending X and Y and constant 0.
REM These are to be used to represent R, G and B values of the darker 3 faces of the cube.

gmt grdmath -I1 -R0/255/0/255 X = x.nc
gmt grdmath -I1 -R Y = y.nc
gmt grdmath -I1 -R 0 = c.nc

gmt gmtset FONT_ANNOT_PRIMARY 12p,Helvetica-Bold

gmt grdimage x.nc y.nc c.nc -JX2.5i/-2.5i -R -K -O -X0.5i >> %ps%
gmt psxy rays.dat -J -R -K -O >> %ps%
echo 128 128 60\217 | gmt pstext -J -R -F+fwhite+a-45 -K -O >> %ps%
echo 102  26 0.4 | gmt pstext -J -R -F+fwhite+a-90 -K -O >> %ps%
echo 204  26 0.8 | gmt pstext -J -R -F+fwhite+a-90 -K -O >> %ps%
echo 10 140 G | gmt pstext -J -R -F+f16p,white+a180 -K -O >> %ps%
echo 0 0 0 128 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> %ps%

gmt grdimage x.nc c.nc y.nc -JX2.5i/2.5i -R -K -O -Y2.5i >> %ps%
gmt psxy rays.dat -J -R -K -O >> %ps%
echo 128 128 300\217 | gmt pstext -J -R -F+fwhite+a45 -K -O >> %ps%
echo 26 102 0.4 | gmt pstext -J -R -F+fwhite -K -O >> %ps%
echo 26 204 0.8 | gmt pstext -J -R -F+fwhite -K -O >> %ps%
echo 140 10 R | gmt pstext -J -R -F+f16p,white+a-90 -K -O >> %ps%
echo 100 100 V | gmt pstext -J -R -F+f16p,white+a-45 -K -O >> %ps%
echo 0 0 128 0 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> %ps%
echo 0 0 90 90 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> %ps%

gmt grdimage c.nc x.nc y.nc -JX-2.5i/2.5i -R -K -O -X-2.5i >> %ps%
gmt psxy rays.dat -J -R -K -O >> %ps%
echo 128 128 180\217 | gmt pstext -J -R -F+fwhite+a135 -K -O >> %ps%
echo 102 26 0.4 | gmt pstext -J -R -F+fwhite+a90 -K -O >> %ps%
echo 204 26 0.8 | gmt pstext -J -R -F+fwhite+a90 -K -O >> %ps%
echo 10 140 B | gmt pstext --FONT=white -J -R -F+f16p,white -K -O >> %ps%
echo 0 0 0 128 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> %ps%
echo 0 0 128 0 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> %ps%

REM Second, create grids of descending X and Y and constant 255.
REM These are to be used to represent R, G and B values of the lighter 3 faces of the cube.

gmt grdmath -I1 -R 255 X SUB = x.nc
gmt grdmath -I1 -R 255 Y SUB = y.nc
gmt grdmath -I1 -R 255       = c.nc

gmt grdimage x.nc y.nc c.nc -JX-2.5i/-2.5i -R -K -O -X2.5i -Y2.5i >> %ps%
gmt psxy rays.dat -J -R -K -O >> %ps%
echo 128 128 240\217 | gmt pstext -J -R -F+a225 -K -O >> %ps%
echo 102 26 0.4 | gmt pstext -J -R -F+a270 -K -O >> %ps%
echo 204 26 0.8 | gmt pstext -J -R -F+a270 -K -O >> %ps%

gmt grdimage c.nc y.nc x.nc -JX2.5i/-2.5i -R -K -O -X2.5i >> %ps%
gmt psxy rays.dat -J -R -K -O >> %ps%
echo 128 128 0\217 | gmt pstext -J -R -F+a-45 -K -O >> %ps%
echo 26 102 0.4 | gmt pstext -J -R -K -O >> %ps%
echo 26 204 0.8 | gmt pstext -J -R -K -O >> %ps%
echo 100 100 S | gmt pstext -J -R -F+f16p+a45 -K -O >> %ps%
echo 204 66 H | gmt pstext -J -R -F+f16p+a90 -K -O >> %ps%
echo 0 0 90 90 | gmt psxy -N -Sv0.15i+s+e -Gblack -W2p -J -R -K -O >> %ps%
echo 204 204 204 76 | gmt psxy -N -Sv0.15i+s+e -Gblack -W2p -J -R -K -O >> %ps%

gmt grdimage x.nc c.nc y.nc -JX-2.5i/2.5i -R -K -O -X-2.5i -Y2.5i >> %ps%
gmt psxy rays.dat -J -R -K -O >> %ps%
echo 128 128 120\217 | gmt pstext -J -R -F+a135 -K -O >> %ps%
echo 26 102 0.4 | gmt pstext -J -R -F+a180 -K -O >> %ps%
echo 26 204 0.8 | gmt pstext -J -R -F+a180 -K -O >> %ps%
echo 200 200 GMT 4 | gmt pstext -J -F+f16p+a225 -R -O >> %ps%

del *.nc
del .gmt*
del gmt.conf
