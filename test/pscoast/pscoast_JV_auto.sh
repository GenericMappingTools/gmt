#!/usr/bin/env bash
# Test Van der Grinten w/e/s/n map regions when NOT setting a central meridian
# Original problem reported in https://github.com/GenericMappingTools/gmt/issues/7282

gmt begin pscoast_JV_auto ps
	gmt subplot begin 4x2 -Fs6c -M2c/2p -T"Van der Grinten unspecified central meridian" -A+o-32p/0 -Y0.5c
		while read minX maxX minY maxY area size
		do
			gmt subplot set -A${area}
			gmt coast -R${minX}/${maxX}/${minY}/${maxY} -JV6c+d${size} -Bg -Dc -Glightgray -Scornsilk -A10000 -Wthinnest
		done <<- EOF
		-180   0   0 90 NW	h
		0 180   0 90 NE	h
		0 180 -90  0 SE	h
		-180   0 -90  0 SW	h
		-180   0 -90 90 W	h
		-180 180   0 90 N	w	
		0 180 -90 90 E	h
		-180 180 -90  0 S	w
		EOF
	gmt subplot end
gmt end show
