#!/bin/bash
gmt coast -Rg -JG4/52/230/90/60/180/60/60/5i -Bx2g2 -By1g1 -Ia -Di -Glightbrown -Wthinnest \
	-Slightblue --MAP_ANNOT_MIN_SPACING=0.25i -ps GMT_perspective
