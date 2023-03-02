#!/usr/bin/env bash
# Test Van der Grinten ew/e/s/n map regions when NOT setting a central meridian
# Original problem reported in https://github.com/GenericMappingTools/gmt/issues/7282

gmt begin pscoast_JV_auto ps
	gmt subplot begin 4x2 -Fs6c -M2c/4p -T"Van der Grinten unspecified central meridian" -A+o-32p/0
		while read minX maxX minY maxY area
		do
			gmt subplot set -A${area}
			gmt coast -R${minX}/${maxX}/${minY}/${maxY} -JV6c+du -Bg -Dc -Glightgray -Scornsilk -A10000 -Wthinnest
		done <<- EOF
		-180   0   0 90 NW
		0 180   0 90 NE
		0 180 -90  0 SE
		-180   0 -90  0 SW
		-180   0 -90 90 W
		-180 180   0 90 N
		0 180 -90 90 E
		-180 180 -90  0 S
		EOF
	gmt subplot end
gmt end show
