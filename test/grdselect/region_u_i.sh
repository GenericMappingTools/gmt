#!/usr/bin/env bash
# Test grdselect for the intersection and union of geographic grids
# One grid is 360 off to test the wrap-around

gmt grdmath -R212/242/0/30     -fg -I2 X = 1.grd
gmt grdmath -R-135/-105/10/40  -fg -I5 X = 2.grd
gmt grdmath -R137W/121W/5S/45N -fg -I1 X = 3.grd

gmt begin region_u_i
	gmt set MAP_FRAME_TYPE plain
	gmt subplot begin 2x1 -Fs15c/0 -R205/285/-10/50 -JQ14c
		gmt grdinfo 1.grd -Ib | gmt plot -A -Glightblue@50 -c -l"212/242/0/30"+H"Intersection"
		gmt grdinfo 2.grd -Ib | gmt plot -A -Glightred@50 -l"-135/-105/10/40"
		gmt grdinfo 3.grd -Ib | gmt plot -A -Glightgreen@50 -l"137W/121W/5S/45N"
		gmt grdselect ?.grd -Ai -Eb | gmt plot -A -W2p
		gmt grdinfo 1.grd -Ib | gmt plot -A -Glightblue@50 -c -l"212/242/0/30"+H"Union w/2@."
		gmt grdinfo 2.grd -Ib | gmt plot -A -Glightred@50 -l"-135/-105/10/40"
		gmt grdinfo 3.grd -Ib | gmt plot -A -Glightgreen@50 -l"137W/121W/5S/45N"
		gmt grdselect ?.grd -Au -M2 -Eb | gmt plot -A -W2p
	gmt subplot end
gmt end show
