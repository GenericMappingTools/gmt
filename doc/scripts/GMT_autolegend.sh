#!/usr/bin/env bash
gmt begin GMT_autolegend
	gmt plot -R0/7.2/3/7.2 -Jx2c @Table_5_11.txt -Sc0.35c -Glightgreen -Wfaint -lApples+h"LEGEND"+f16p+d
	gmt plot @Table_5_11.txt -St0.35c -Gorange -B -BWStr -lOranges
	gmt legend -DjTR+w3c+o0.25c -F+p1p+ggray95+s
gmt end show
