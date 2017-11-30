#!/bin/bash
#		GMT EXAMPLE 11
#		$Id$
#
# Purpose:	Create a 3-D RGB Cube
# GMT modules:	gmtset, grdimage, grdmath, pstext, psxy
# Unix progs:	rm
ps=example_11.ps

# Use gmt psxy to plot "cut-along-the-dotted" lines.

gmt set MAP_TICK_LENGTH_PRIMARY 0

gmt psxy @cut-here_11.txt -Wthinnest,. -R-51/306/0/1071 -JX3.5i/10.5i -X2.5i -Y0.5i -P -K > $ps

# First, create grids of ascending X and Y and constant 0.
# These are to be used to represent R, G and B values of the darker 3 faces of the cube.

gmt grdmath -I1 -R0/255/0/255 X = x.nc
gmt grdmath -I1 -R Y = y.nc
gmt grdmath -I1 -R 0 = c.nc

gmt set FONT_ANNOT_PRIMARY 12p,Helvetica-Bold

gmt grdimage x.nc y.nc c.nc -JX2.5i/-2.5i -R -K -O -X0.5i >> $ps
gmt psxy -Wthinner,white,- @rays_11.txt -J -R -K -O >> $ps
gmt pstext --FONT=white -J -R -K -O -F+a+f >> $ps << END
128 128 -45 12p 60\217
102  26 -90 12p 0.4
204  26 -90 12p 0.8
10  140 180 16p G
END
echo 0 0 0 128 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> $ps

gmt grdimage x.nc c.nc y.nc -JX2.5i/2.5i -R -K -O -Y2.5i >> $ps
gmt psxy -Wthinner,white,- @rays_11.txt -J -R -K -O >> $ps
gmt pstext --FONT=white -J -R -K -O -F+a+f >> $ps << END
128 128  45 12p 300\217
26  102   0 12p 0.4
26  204   0 12p 0.8
140  10 -90 16p R
100 100 -45 16p V
END
echo 0 0 128 0 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> $ps
echo 0 0 90 90 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> $ps

gmt grdimage c.nc x.nc y.nc -JX-2.5i/2.5i -R -K -O -X-2.5i >> $ps
gmt psxy -Wthinner,white,- @rays_11.txt -J -R -K -O >> $ps
gmt pstext --FONT=white -J -R -K -O -F+a+f >> $ps << END
128 128 135 12p 180\217
102  26  90 12p 0.4
204  26  90 12p 0.8
10  140   0 16p B
END
echo 0 0 0 128 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> $ps
echo 0 0 128 0 | gmt psxy -N -Sv0.15i+s+e -Gwhite -W2p,white -J -R -K -O >> $ps

# Second, create grids of descending X and Y and constant 255.
# These are to be used to represent R, G and B values of the lighter 3 faces of the cube.

gmt grdmath -I1 -R 255 X SUB = x.nc
gmt grdmath -I1 -R 255 Y SUB = y.nc
gmt grdmath -I1 -R 255       = c.nc

gmt grdimage x.nc y.nc c.nc -JX-2.5i/-2.5i -R -K -O -X2.5i -Y2.5i >> $ps
gmt psxy -Wthinner,black,- @rays_11.txt -J -R -K -O >> $ps
gmt pstext -J -R -K -O -F+a+f >> $ps << END
128 128 225 12p 240\217
102  26 270 12p 0.4
204  26 270 12p 0.8
END

gmt grdimage c.nc y.nc x.nc -JX2.5i/-2.5i -R -K -O -X2.5i >> $ps
gmt psxy -Wthinner,black,- @rays_11.txt -J -R -K -O >> $ps
gmt pstext -J -R -K -O -F+a+f >> $ps << END
128 128 -45 12p 0\217
26  102   0 12p 0.4
26  204   0 12p 0.8
100 100  45 16p S
204  66  90 16p H
END
echo 0 0 90 90 | gmt psxy -N -Sv0.15i+s+e -Gblack -W2p -J -R -K -O >> $ps
echo 204 204 204 76 | gmt psxy -N -Sv0.15i+s+e -Gblack -W2p -J -R -K -O >> $ps

gmt grdimage x.nc c.nc y.nc -JX-2.5i/2.5i -R -K -O -X-2.5i -Y2.5i >> $ps
gmt psxy -Wthinner,black,- @rays_11.txt -J -R -K -O >> $ps
gmt pstext -J -R -O -F+a+f >> $ps << END
128 128 135 12p 120\217
26  102 180 12p 0.4
26  204 180 12p 0.8
200 200 225 16p GMT
END

rm -f *.nc gmt.conf
