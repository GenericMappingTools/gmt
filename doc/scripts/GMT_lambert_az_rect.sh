#!/usr/bin/env bash
gmt set FORMAT_GEO_MAP ddd:mm:ssF MAP_GRID_CROSS_SIZE_PRIMARY 0
gmt coast -R0/-40/60/-10r -JA30/-30/4.5i -Bag -Dl -A500 -Gp10+r300 -Wthinnest -ps GMT_lambert_az_rect
