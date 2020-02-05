#!/usr/bin/env bash
# Make a series of 4 spikes on a grid, then create a variable
# filter width grid with 4 quandrants of different widths
gmt begin varfilter ps
	# Make the data:
	gmt grdmath -R-20/20/-20/20 -I0.25 0 = a.grd
	gmt grdmath -R-10/10/-10/10 -I20 1 = b.grd
	gmt grd2xyz a.grd > tmp
	gmt grd2xyz b.grd >> tmp
	gmt xyz2grd -R-20/20/-20/20 -I0.25 tmp -Az -Gin.grd
	# Make the filter width grid
	gmt grdmath -R-20/20/-20/20 -I0.25 X 0 GE 2 MUL Y 0 GE ADD 1 ADD 5 MUL = fw.grd
	# Make the plot
	gmt subplot begin 2x2 -Fs3i -SR -SC -T"Variable Filter Width" -M0.5c
	gmt makecpt -T0/1 -Cwhite,black -H > t.cpt
	gmt makecpt -T0/0.005 -Cjet -H > f.cpt
	gmt grdimage in.grd -c0 -Ct.cpt
	gmt basemap -c1
	gmt grdimage fw.grd -Cjet -t50
	gmt text -F+f36p<<- EOF
	-10 -10 5
	+10 -10 15
	-10 +10 10
	+10 +10 20
	EOF
	gmt grdfilter in.grd -Fg20 -Gg.grd -D0
	gmt grdimage g.grd  -c2 -Cf.cpt
	gmt grdfilter in.grd -Fgfw.grd -Gfg.grd -D0
	gmt grdimage fg.grd  -c3  -Cf.cpt
	gmt subplot end
gmt end show
rm -f a.grd b.grd tmp in.grd fw.grd t.cpt f.cpt
