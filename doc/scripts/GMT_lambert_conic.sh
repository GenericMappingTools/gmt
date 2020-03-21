#!/usr/bin/env bash
gmt begin GMT_lambert_conic
	gmt set MAP_FRAME_TYPE FANCY FORMAT_GEO_MAP ddd:mm:ssF MAP_GRID_CROSS_SIZE_PRIMARY 0.15c
	gmt coast -R-130/-70/24/52 -Jl-100/35/33/45/1:50000000 -Bag -Dl -N1/thick,red -N2/thinner -A500 -Gtan -Wthinnest,white -Sblue
gmt end show
