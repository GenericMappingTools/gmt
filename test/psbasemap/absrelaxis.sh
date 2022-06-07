#!/usr/bin/env bash
# Test the 3 frames in https://forum.generic-mapping-tools.org/t/incoherent-gridline-behaviour/1319/13
gmt begin absrelaxis ps
	gmt set MAP_GRID_PEN_PRIMARY thinner,grey MAP_GRID_PEN_SECONDARY thin,black TIME_INTERVAL_FRACTION 0.1
	gmt basemap -R110/280/373420800/381369600 -JX16cd/8ct -Bxa20f10g10 -Bpyf1Og1O -Bsya1Yf1Yg1Y -BWSen -Xc -Y1c
	gmt basemap -JX16cd/8cT -Bxa20f10g10 -Bpyf1Og1O -Bsya1Yf1Yg1Y -BWSen -Y9c
	gmt basemap -R110/280/1981-11-01T/1982-02-01T -JX16cd/8cT -Bxa20f10g10 -Bpyf1Og1O -Bsya1Yf1Yg1Y -BWSen -Y9c
gmt end show
