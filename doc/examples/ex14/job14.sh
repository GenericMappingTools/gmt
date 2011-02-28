#!/bin/bash
#		GMT EXAMPLE 14
#		$Id: job14.sh,v 1.19 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Showing simple gridding, contouring, and resampling along tracks
# GMT progs:	blockmean, grdcontour, grdtrack, grdtrend, minmax, project
# GMT progs:	gmtset, pstext, psbasemap, psxy, surface
# Unix progs:	$AWK, rm
#
. ../functions.sh
ps=../example_14.ps

# First draw network and label the nodes

gmtset GRID_PEN_PRIMARY thinnest,-
psxy table_5.11 -R0/7/0/7 -JX3.06i/3.15i -B2f1WSNe -Sc0.05i -Gblack -P -K -Y6.45i > $ps
$AWK '{printf "%g %s 6 0 0 LM %g\n", $1+0.08, $2, $3}' table_5.11 | pstext -R -J -O -K -N >> $ps
blockmean table_5.11 -R0/7/0/7 -I1 > mean.xyz

# Then draw blockmean cells

psbasemap -R0.5/7.5/0.5/7.5 -J -O -K -B0g1 -X3.25i >> $ps
psxy -R0/7/0/7 -J -B2f1eSNw mean.xyz -Ss0.05i -Gblack -O -K >> $ps
$AWK '{printf "%g %s 6 0 0 LM %g\n", $1+0.1, $2, $3}' mean.xyz \
	| pstext -R -J -O -K -Wwhite,o -C0.01i/0.01i -N >> $ps

# Then surface and contour the data

surface mean.xyz -R -I1 -Gdata.nc
grdcontour data.nc -J -B2f1WSne -C25 -A50 -G3i/10 -S4 -O -K -X-3.25i -Y-3.55i >> $ps
psxy -R -J mean.xyz -Ss0.05i -Gblack -O -K >> $ps

# Fit bicubic trend to data and compare to gridded surface

grdtrend data.nc -N10 -Ttrend.nc
project -C0/0 -E7/7 -G0.1 > track
grdcontour trend.nc -J -B2f1wSne -C25 -A50 -Glct/cb -S4 -O -K -X3.25i >> $ps
psxy -R -J track -Wthick,. -O -K >> $ps

# Sample along diagonal

grdtrack track -Gdata.nc | cut -f3,4 > data.d
grdtrack track -Gtrend.nc | cut -f3,4 > trend.d
psxy `minmax data.d trend.d -I0.5/25` -JX6.3i/1.4i data.d -Wthick -O -K -X-3.25i -Y-1.9i \
	-B1/50WSne >> $ps
psxy -R -J trend.d -Wthinner,- -O -U"Example 14 in Cookbook" >> $ps

rm -f mean.xyz track *.nc *.d .gmt*
