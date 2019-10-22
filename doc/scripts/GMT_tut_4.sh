#!/usr/bin/env bash
gmt begin GMT_tut_4
	gmt coast -R-130/-70/24/52 -JB-100/35/33/45/6i -B -B+t"Conic Projection" -N1/thickest -N2/thinnest -A500 -Ggray -Wthinnest
gmt end show
