#!/usr/bin/env bash
gmt begin GMT_linear_d
	gmt set MAP_GRID_CROSS_SIZE_PRIMARY 0.1i MAP_FRAME_TYPE FANCY FORMAT_GEO_MAP ddd:mm:ssF
	gmt coast -Rg-55/305/-90/90 -Jx0.04c -Bagf -BWSen -Dc -A1000 -Glightbrown -Wthinnest -Slightblue
gmt end show
