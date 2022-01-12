#!/usr/bin/env bash
# Test automated contour legend
# DVC_TEST

gmt begin contourlegend ps
	gmt grdcontour @earth_relief_20m -R0/20/0/30 -JM6i -A1000 -C500 -Baf -Wc1p,red -Wa1p,blue -l"Annotated Contour"/"Regular Contour"+H"LEGEND"+D
gmt end show
