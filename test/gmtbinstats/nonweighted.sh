#!/usr/bin/env bash
# Test most operators in gmtbinstats for circular binning, without weights
gmt begin nonweighted ps
	gmt set FONT_TAG 12p MAP_FRAME_TYPE plain
		gmt subplot begin 7x2 -Fs3i/1.25i -M15p/5p -A+gwhite -Scb -Srl -Bwstr -R-110/250/-75/75  -JQ70E/3i -X1.25i -Y0.5i
		# Plot data
		gmt subplot set 0 -Adata
		gmt makecpt -T0/10000000 -Cjet
		gmt coast -Glightgray
		gmt plot @capitals.gmt -a2=population -Sc0.1c -C
		# Plot mean population
		gmt subplot set 1 -Aa
		gmt binstats -I5 @capitals.gmt -a2=population -Ca -Gt.grd -S2000k
		gmt grdimage t.grd -JQ70E/3i
		# Plot MAD population
		gmt subplot set 2 -Ad
		gmt binstats -I5 @capitals.gmt -a2=population -Cd -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot stdev population
		gmt subplot set 3 -As
		gmt binstats -I5 @capitals.gmt -a2=population -Cs -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot inter-quartile range population
		gmt subplot set 4 -Ai
		gmt binstats -I5 @capitals.gmt -a2=population -Ci -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot minimum population
		gmt subplot set 5 -Al
		gmt binstats -I5 @capitals.gmt -a2=population -Cl -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot median population
		gmt subplot set 6 -Am
		gmt binstats -I5 @capitals.gmt -a2=population -Cm -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot number of points
		gmt subplot set 7 -An
		gmt binstats -I5 @capitals.gmt -Cn -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot LMSscale population
		gmt subplot set 8 -Ao
		gmt binstats -I5 @capitals.gmt -a2=population -Co -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot mode population
		gmt subplot set 9 -Ap
		gmt binstats -I5 @capitals.gmt -a2=population -Cp -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot quantile 75 population
		gmt subplot set 10 -Aq75
		gmt binstats -I5 @capitals.gmt -a2=population -Cq75 -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot range population
		gmt subplot set 11 -Ag
		gmt binstats -I5 @capitals.gmt -a2=population -Cg -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot sum population
		gmt subplot set 12 -Az
		gmt binstats -I5 @capitals.gmt -a2=population -Cz -Gt.grd -S2000k
		gmt grdimage t.grd
		# Plot maximum population
		gmt subplot set 13 -Au
		gmt binstats -I5 @capitals.gmt -a2=population -Cu -Gt.grd -S2000k
		gmt grdimage t.grd
	gmt subplot end
gmt end show
