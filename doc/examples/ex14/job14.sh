#!/bin/sh
#		GMT EXAMPLE 14
#
#		$Id: job14.sh,v 1.2 2002-01-30 03:40:56 ben Exp $
#
# Purpose:	Showing simple gridding, contouring, and resampling along tracks
# GMT progs:	blockmean, grdcontour, grdtrack, grdtrend, minmax, project, pstext
#		psbasemap, psxy, surface
# Unix progs:	$AWK, rm
#
# First draw network and label the nodes
gmtset GRID_PEN 0.25pta
psxy table_5.11 -R0/7/0/7 -JX3.06i/3.15i -B2f1WSNe -Sc0.05i -G0 -P -K -Y6.45i > example_14.ps
$AWK '{printf "%g %s 6 0 0 LM %g\n", $1+0.08, $2, $3}' table_5.11 | pstext -R -JX -O -K -N >> example_14.ps
blockmean table_5.11 -R0/7/0/7 -I1 > mean.xyz
# Then draw blocmean cells
psbasemap -R0.5/7.5/0.5/7.5 -JX -O -K -B0g1 -X3.25i >> example_14.ps
psxy -R -JX -B2f1eSNw mean.xyz -Ss0.05i -G0 -O -K >> example_14.ps
$AWK '{printf "%g %s 6 0 0 LM %g\n", $1+0.1, $2, $3}' mean.xyz | pstext -R -JX -O -K -W255o -C0.01i/0.01i -N >> example_14.ps
# Then surface and contour the data
surface mean.xyz -R -I1 -Gdata.grd
grdcontour data.grd -JX -B2f1WSne -C25 -A50 -G3i/10 -S4 -O -K -X-3.25i -Y-3.55i >> example_14.ps
psxy -R -JX mean.xyz -Ss0.05i -G0 -O -K >> example_14.ps
# Fit bicubic trend to data and compare to gridded surface
grdtrend data.grd -N10 -Ttrend.grd
grdcontour trend.grd -JX -B2f1wSne -C25 -A50 -G3i/10 -S4 -O -K -X3.25i >> example_14.ps
project -C0/0 -E7/7 -G0.1 -Fxy > track
psxy -R -JX track -W1pto -O -K >> example_14.ps
# Sample along diagonal
grdtrack track -Gdata.grd | cut -f3,4 > data.d
grdtrack track -Gtrend.grd | cut -f3,4 > trend.d
psxy `minmax data.d trend.d -I0.5/25` -JX6.3i/1.4i data.d -W1p -O -K -X-3.25i -Y-1.9i -B1/50WSne >> example_14.ps
psxy -R -JX trend.d -W0.5pta -O -U"Example 14 in Cookbook" >> example_14.ps
\rm mean.xyz track *.grd *.d .gmtcommands
