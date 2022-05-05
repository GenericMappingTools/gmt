#!/usr/bin/env bash
#
# Illustrate the functions of -Wr and -We in mapproject and
# compare the enclosed versus circumescribed oblique retions

gmt begin GMT_obl_regions
	gmt set MAP_FRAME_TYPE plain MAP_ANNOT_OBLIQUE lat_horizontal,tick_normal,lon_horizontal FONT_ANNOT 8p
	# Oblique Mercator region defined - find enclosing w/e/s/n rectangle
	R=$(gmt mapproject -R270/20/305/25+r -JOc280/25.5/22/69/5c+dh -WR)
	gmt mapproject -Wr+n > geo.txt
	gmt coast $R -JM5c+dh -Bafg -Glightgray
	gmt clip geo.txt -W1p,red
	gmt coast -Gblack
	gmt clip -C
	# Stereographic region defined - find enclosing mn/max oblique rectangle
	R=$(gmt mapproject -JS36/90/5c+dh -R-15/60/68/90 -WE)
	cat <<- EOF > geo.txt	# Build the geographic box
	-15	68
	60	68
	60	90
	-15	90
	EOF
	gmt coast $R -Bxa15f5g10 -Bya5f5g5 -Glightgray -X7.5c --MAP_FRAME_PEN=1p,red
	gmt clip geo.txt -W1p -Ap
	gmt coast -Gblack
	gmt clip -C -Ap
gmt end show
