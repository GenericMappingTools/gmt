#!/bin/bash
#	$Id$
# Showing map directional roses
ps=GMT_dir_rose.ps
gmt set FONT_LABEL 10p FONT_TITLE 12p MAP_ANNOT_OBLIQUE 34 MAP_TITLE_OFFSET 5p \
	MAP_FRAME_WIDTH 3p FORMAT_GEO_MAP dddF FONT_ANNOT_PRIMARY 10p
# left: Fancy kind = 1
gmt psbasemap -R-5/5/-5/5 -Jm0.15i -Ba5f -BWSne+gazure1 -Tdg0/0+w1i -P -K -X1i > $ps
# middle: Fancy kind = 3
gmt psbasemap -R -J -Ba5f -BwSne+gazure1 -Tdg0/0+w1i+f1 -O -K -X1.75i >> $ps
# right: Plain kind
gmt psbasemap -R-7/7/-5/5 -J -Ba5f -BwSnE+gazure1 -Tdg0/0+w1i+f3+l -O -X1.75i >> $ps
