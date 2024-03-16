#!/usr/bin/env bash
# Test the -L option in surface for both constants and grids
#
gmt begin limits
	gmt subplot begin 3x2 -R0/6.3/0/6.3 -Fs7c -A+gwhite -Sct -Srl
	gmt subplot set -A"data"
	gmt plot @Table_5_11.txt -Sc2p -Gblack
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -Gno_limits.grd
	gmt subplot set -A"no limit"
	gmt grdcontour no_limits.grd -C10 -A50 -GlTC/BC
	gmt plot @Table_5_11.txt -Sc2p -Gblack
	gmt subplot set -A"low = 749"
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -GLl_limits.grd -Ll749
	gmt plot @Table_5_11.txt -Sc2p -Gblack
	gmt grdcontour Ll_limits.grd -C10 -A50 -GlTC/BC
	gmt subplot set -A"high = 901"
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -GLu_limits.grd -Lu901
	gmt grdcontour Lu_limits.grd -C10 -A50 -GlTC/BC
	gmt plot @Table_5_11.txt -Sc2p -Gblack
	gmt subplot set -A"low = grid"
	gmt grdmath -R0/6.3/0/6.3 -I0.1 749 = L.grd
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -GLl_grid.grd -LlL.grd
	gmt grdcontour Ll_grid.grd -C10 -A50 -GlTC/BC
	gmt plot @Table_5_11.txt -Sc2p -Gblack
	gmt subplot set -A"high = grid"
	gmt grdmath -R0/6.3/0/6.3 -I0.1 901 = H.grd
	gmt surface @Table_5_11.txt -R0/6.3/0/6.3 -I0.1 -GLu_grid.grd -LuH.grd
	gmt grdcontour Lu_grid.grd -C10 -A50 -GlTC/BC
	gmt plot @Table_5_11.txt -Sc2p -Gblack
	gmt subplot end
gmt end show
