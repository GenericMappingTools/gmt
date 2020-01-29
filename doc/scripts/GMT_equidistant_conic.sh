#!/usr/bin/env bash
gmt begin GMT_equidistant_conic
	gmt set FORMAT_GEO_MAP ddd:mm:ssF MAP_GRID_CROSS_SIZE_PRIMARY 0.15c
	gmt coast -R-88/-70/18/24 -JD-79/21/19/23/12c -Bag -Di -N1/thick,red -Glightgreen -Wthinnest
gmt end show
