#!/bin/bash
#		GMT EXAMPLE 11
#		$Id: job11.sh,v 1.15 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Create a 3-D RGB Cube
# GMT progs:	gmtset, grdimage, grdmath, pstext, psxy
# Unix progs:	rm
. ../functions.sh
ps=../example_11.ps

# Use psxy to plot "cut-along-the-dotted" lines.

gmtset TICK_LENGTH 0 COLOR_MODEL rgb CHAR_ENCODING Standard+

psxy cut-here.dat -Wthinnest,. -m -R-51/306/0/1071 -JX3.5i/10.5i -X2.5i -Y0.5i \
	-P -U/-2.0i/-0.2i/"Example 11 in Cookbook" -K > $ps

# First, create grids of ascending X and Y and constant 0.
# These are to be used to represent R, G and B values of the darker 3 faces of the cube.

grdmath -I1 -R0/255/0/255 X = x.nc
grdmath -I1 -R Y = y.nc
grdmath -I1 -R 0 = c.nc

grdimage x.nc y.nc c.nc -JX2.5i/-2.5i -R -K -O -X0.5i >> $ps
psxy -m -Wthinner,white,- rays.dat -J -R -K -O -Bwesn >> $ps
echo 128 128 12 -45 1 MC "60\217" | pstext -Gwhite -J -R -K -O >> $ps
echo 102 26 12 -90 1 MC 0.4 | pstext -Gwhite -J -R -K -O >> $ps
echo 204 26 12 -90 1 MC 0.8 | pstext -Gwhite -J -R -K -O >> $ps
echo 10 140 16 180 1 MC G | pstext -Gwhite -J -R -K -O >> $ps
echo 0 0 0 128 | psxy -N -Svs -Gwhite -J -R -K -O >> $ps

grdimage x.nc c.nc y.nc -JX2.5i/2.5i -R -K -O -Y2.5i >> $ps
psxy -m -Wthinner,white,- rays.dat -J -R -K -O -Bwesn >> $ps
echo 128 128 12 45 1 MC "300\217" | pstext -Gwhite -J -R -K -O >> $ps
echo 26 102 12 0 1 MC 0.4 | pstext -Gwhite -J -R -K -O >> $ps
echo 26 204 12 0 1 MC 0.8 | pstext -Gwhite -J -R -K -O >> $ps
echo 140 10 16 -90 1 MC R | pstext -Gwhite -J -R -K -O >> $ps
echo 100 100 16 -45 1 MC V | pstext -Gwhite -J -R -K -O >> $ps
echo 0 0 128 0 | psxy -N -Svs -Gwhite -J -R -K -O >> $ps
echo 0 0 90 90 | psxy -N -Svs -Gwhite -J -R -K -O >> $ps

grdimage c.nc x.nc y.nc -JX-2.5i/2.5i -R -K -O -X-2.5i >> $ps
psxy -m -Wthinner,white,- rays.dat -J -R -K -O -Bwesn >> $ps
echo 128 128 12 135 1 MC "180\217" | pstext -Gwhite -J -R -K -O >> $ps
echo 102 26 12 90 1 MC 0.4 | pstext -Gwhite -J -R -K -O >> $ps
echo 204 26 12 90 1 MC 0.8 | pstext -Gwhite -J -R -K -O >> $ps
echo 10 140 16 0 1 MC B | pstext -Gwhite -J -R -K -O >> $ps
echo 0 0 0 128 | psxy -N -Svs -Gwhite -J -R -K -O >> $ps
echo 0 0 128 0 | psxy -N -Svs -Gwhite -J -R -K -O >> $ps

# Second, create grids of descending X and Y and constant 255.
# These are to be used to represent R, G and B values of the lighter 3 faces of the cube.

grdmath -I1 -R 255 X SUB = x.nc
grdmath -I1 -R 255 Y SUB = y.nc
grdmath -I1 -R 255       = c.nc

grdimage x.nc y.nc c.nc -JX-2.5i/-2.5i -R -K -O -X2.5i -Y2.5i >> $ps
psxy -m -Wthinner,black,- rays.dat -J -R -K -O -Bwesn >> $ps
echo 128 128 12 225 1 MC "240\217" | pstext -J -R -K -O >> $ps
echo 102 26 12 270 1 MC 0.4 | pstext -J -R -K -O >> $ps
echo 204 26 12 270 1 MC 0.8 | pstext -J -R -K -O >> $ps

grdimage c.nc y.nc x.nc -JX2.5i/-2.5i -R -K -O -X2.5i >> $ps
psxy -m -Wthinner,black,- rays.dat -J -R -K -O -Bwesn >> $ps
echo 128 128 12 -45 1 MC "0\217" | pstext -J -R -K -O >> $ps
echo 26 102 12 0 1 MC 0.4 | pstext -J -R -K -O >> $ps
echo 26 204 12 0 1 MC 0.8 | pstext -J -R -K -O >> $ps
echo 100 100 16 45 1 MC S | pstext -Gblack -J -R -K -O >> $ps
echo 204 66 16 90 1 MC H | pstext -Gblack -J -R -K -O >> $ps
echo 0 0 90 90 | psxy -N -Svs -Gblack -J -R -K -O >> $ps
echo 204 204 204 76 | psxy -N -Svs -Gblack -J -R -K -O >> $ps

grdimage x.nc c.nc y.nc -JX-2.5i/2.5i -R -K -O -X-2.5i -Y2.5i >> $ps
psxy -m -Wthinner,black,- rays.dat -J -R -K -O -Bwesn >> $ps
echo 128 128 12 135 1 MC "120\217" | pstext -J -R -K -O >> $ps
echo 26 102 12 180 1 MC 0.4 | pstext -J -R -K -O >> $ps
echo 26 204 12 180 1 MC 0.8 | pstext -J -R -K -O >> $ps
echo 200 200 16 225 1 MC GMT 4 | pstext -J -R -O >> $ps

rm -f *.nc .gmt*
