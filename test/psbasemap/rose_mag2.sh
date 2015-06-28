#!/bin/bash
#	$Id$
# Testing map directional roses for magnetics
ps=rose_mag2.ps
gmt set FONT_ANNOT_PRIMARY 9p FONT_ANNOT_SECONDARY 12p FONT_LABEL 14p FONT_TITLE 24p \
	MAP_TITLE_OFFSET 7p MAP_ANNOT_OBLIQUE 34 MAP_FRAME_WIDTH 3p \
	MAP_VECTOR_SHAPE 0.5 MAP_TICK_PEN_SECONDARY thinner,red MAP_TICK_PEN_PRIMARY thinner,blue
# 2nd row: Magnetic rose with a specified declination
gmt psbasemap -R-10/-2.5/10/2.5r -JOc0/0/50/60/5.8i -Baf -BWSne -P -K -X1.25i --FONT_ANNOT_PRIMARY=12p > $ps
gmt psbasemap -R -J -Tmg0/0+w2.5i+d-14.5+i45/10/5 -O -K >> $ps
# 1st row: Magnetic rose with unspecified declination
gmt psbasemap -R -J -Baf -BWsne -O -K -Y4.7i --FONT_ANNOT_PRIMARY=12p >> $ps
gmt psbasemap -R -J -Tmg0/0+w2.5i+i45/10/5 -O --MAP_TICK_LENGTH_PRIMARY=8p/4p >> $ps
