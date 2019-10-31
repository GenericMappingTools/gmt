#!/usr/bin/env bash
#		GMT EXAMPLE 14
#
# Purpose:	Showing simple gridding, contouring, and resampling along tracks
# GMT modules:	blockmean, grdcontour, grdtrack, grdtrend, project, text,
# GMT modules:	set, plot, surface, subplot
# Unix progs:	rm
#
gmt begin ex14
	gmt set MAP_GRID_PEN_PRIMARY thinnest,-
	# calculate mean data and grids
	gmt blockmean @Table_5_11.txt -R0/7/0/7 -I1 > mean.xyz
	gmt surface mean.xyz -Gdata.nc
	gmt grdtrend data.nc -N10 -Ttrend.nc
	gmt project -C0/0 -E7/7 -G0.1 -N > track
	# Sample along diagonal
	gmt grdtrack track -Gdata.nc -o2,3 > data.d
	gmt grdtrack track -Gtrend.nc -o2,3 > trend.d
	gmt plot -Ra -JX6i/1.4i data.d -Wthick -Bx1 -By50 -BWSne
	gmt plot trend.d -Wthinner,-
	gmt subplot begin 2x2 -M0.05i -Ff6i/6i -BWSne -Yh+0.4i
		# First draw network and label the nodes
		gmt plot @Table_5_11.txt -R0/7/0/7 -Sc0.05i -Gblack -c0,0
		gmt text @Table_5_11.txt -D0.1c/0 -F+f6p+jLM -N
		# Then draw gmt blockmean cells and label data values using one decimal
		gmt plot mean.xyz -Ss0.05i -Gblack -c0,1
		gmt text -D0.15c/0 -F+f6p+jLM+z%.1f -Gwhite -W -C0.01i -N mean.xyz
		# Then gmt surface and contour the data
		gmt grdcontour data.nc -C25 -A50 -Gd3i -S4 -c1,0
		gmt plot mean.xyz -Ss0.05i -Gblack
		# Fit bicubic trend to data and compare to gridded gmt surface
		gmt grdcontour trend.nc -C25 -A50 -Glct/cb -S4 -c1,1
		gmt plot track -Wthick,.
	gmt subplot end
gmt end show
rm -f mean.xyz track trend.nc data.nc data.d trend.d
