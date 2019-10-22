#!/usr/bin/env bash
# Showing map directional roses
gmt begin GMT_dir_rose
	gmt set FONT_LABEL 10p FONT_TITLE 12p MAP_ANNOT_OBLIQUE 34 MAP_TITLE_OFFSET 5p \
		MAP_FRAME_WIDTH 3p FORMAT_GEO_MAP dddF FONT_ANNOT_PRIMARY 10p
# left: Fancy kind = 1
	gmt basemap -R-5/5/-5/5 -Jm0.15i -Ba5f -BWSne+gazure1 -Tdg0/0+w1i+jCM -X1i
# middle: Fancy kind = 3
	gmt basemap -Ba5f -BwSne+gazure1 -Tdg0/0+w1i+f1+jCM+l,,,N -X1.75i
# right: Plain kind
	gmt basemap -R-7/7/-5/5 -Ba5f -BwSnE+gazure1 -Tdg0/0+w1i+f3+l+jCM -X1.75i
gmt end show
