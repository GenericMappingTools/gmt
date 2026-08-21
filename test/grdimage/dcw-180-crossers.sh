#!/usr/bin/env bash
# Ensure that the region for the US, Russia and Fiji are sensible
# when accessing tiles, i.e., 5m and higher resolutions.
# and that maps are made. Issue only affects tiles.
gmt begin dcw-180-crossers ps
	gmt grdimage @earth_relief_05m -Bafg180 -RUS
	gmt grdimage @earth_relief_05m -Bafg180 -RRU -Y7.5c
	gmt grdimage @earth_relief_05m -Bafg180 -RFJ -Y4.5c
gmt end show
