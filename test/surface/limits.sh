#!/usr/bin/env bash
# Test the -L option in surface
gmt begin limits
	gmt subplot begin 3x2 -R0/6.3/0/6.3 -Fs8c -A+gwhite -Sct -Srl
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -Gno_limits.grd
	gmt subplot set 0 -A"no limit"
	gmt grdcontour no_limits.grd -C10 -A50
	gmt subplot set 1 -A"low = 750"
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -GLl_limits.grd -Ll750 -Lu930
	gmt grdcontour Ll_limits.grd -C10 -A50
	gmt subplot set 2 -A"high = 900"
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -GLu_limits.grd -Lu900
	gmt grdcontour Lu_limits.grd -C10 -A50
	gmt subplot set 3 -A"low = grid"
	gmt grdmath -R0/6.3/0/6.3 -I0.1 750 = L.grd
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -GLl_grid.grd -LlL.grd
	gmt grdcontour Ll_grid.grd -C10 -A50
	gmt subplot set 4 -A"high = grid"
	gmt grdmath -R0/6.3/0/6.3 -I0.1 900 = H.grd
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -GLu_grid.grd -LuH.grd
	gmt grdcontour Lu_grid.grd -C10 -A50
	gmt subplot end
gmt end show
