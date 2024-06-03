#!/usr/bin/env bash
gmt begin GMT_albers
	gmt set MAP_GRID_CROSS_SIZE_PRIMARY 0
	gmt coast -R110/140/20/35 -JB125/20/25/45/12c -Bag -Dl -Ggreen -Wthinnest -A250
gmt end show
