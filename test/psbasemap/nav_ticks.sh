#!/usr/bin/env bash
# Check the centered and asymmetrical gridline ticks for negative grid cross sizes on an azimuthal projection
gmt begin nav_ticks ps
  gmt basemap -Rg -JG8.1703/49.9816/18c -Bg
  gmt basemap -Bsg5d -Bpg1d --MAP_GRID_CROSS_SIZE_PRIMARY=-3p --MAP_GRID_CROSS_SIZE_SECONDARY=+5p --MAP_GRID_PEN_PRIMARY=default,blue --MAP_GRID_PEN_SECONDARY=default,red
gmt end show
