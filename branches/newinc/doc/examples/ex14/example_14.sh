#!/bin/bash
#		GMT EXAMPLE 14
#		$Id$
#
# Purpose:	Showing simple gridding, contouring, and resampling along tracks
# GMT progs:	blockmean, grdcontour, grdtrack, grdtrend, gmtinfo, project
# GMT progs:	gmtset, pstext, psbasemap, psxy, surface
# Unix progs:	rm
#
ps=example_14.ps

# First draw network and label the nodes

gmt gmtset MAP_GRID_PEN_PRIMARY thinnest,-
gmt psxy table_5.11 -R0/7/0/7 -JX3.06i/3.15i -B2f1 -BWSNe -Sc0.05i -Gblack -P -K -Y6.45i > $ps
gmt pstext table_5.11 -R -J -D0.1c/0 -F+f6p+jLM -O -K -N >> $ps
gmt blockmean table_5.11 -R0/7/0/7 -I1 > mean.xyz

# Then draw gmt blockmean cells

gmt psbasemap -R0.5/7.5/0.5/7.5 -J -O -K -Bg1 -X3.25i >> $ps
gmt psxy -R0/7/0/7 -J -B2f1 -BeSNw mean.xyz -Ss0.05i -Gblack -O -K >> $ps
# Reformat to one decimal for annotation purposes
gmt gmtconvert mean.xyz --FORMAT_FLOAT_OUT=%.1f | \
	gmt pstext -R -J -D0.15c/0 -F+f6p+jLM -O -K -Gwhite -W -C0.01i -N >> $ps

# Then gmt surface and contour the data

gmt surface mean.xyz -R -I1 -Gdata.nc
gmt grdcontour data.nc -J -B2f1 -BWSne -C25 -A50 -Gd3i -S4 -O -K -X-3.25i -Y-3.55i >> $ps
gmt psxy -R -J mean.xyz -Ss0.05i -Gblack -O -K >> $ps

# Fit bicubic trend to data and compare to gridded gmt surface

gmt grdtrend data.nc -N10 -Ttrend.nc
gmt project -C0/0 -E7/7 -G0.1 -N > track
gmt grdcontour trend.nc -J -B2f1 -BwSne -C25 -A50 -Glct/cb -S4 -O -K -X3.25i >> $ps
gmt psxy -R -J track -Wthick,. -O -K >> $ps

# Sample along diagonal

gmt grdtrack track -Gdata.nc -o2,3 > data.d
gmt grdtrack track -Gtrend.nc -o2,3 > trend.d
gmt psxy `gmt info data.d trend.d -I0.5/25` -JX6.3i/1.4i data.d -Wthick -O -K -X-3.25i -Y-1.9i \
	-Bx1 -By50 -BWSne >> $ps
gmt psxy -R -J trend.d -Wthinner,- -O >> $ps

rm -f mean.xyz track *.nc *.d gmt.conf
