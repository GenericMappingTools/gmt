#!/usr/bin/env bash
gmt begin GMT_mag_rose
# Magnetic rose with a specified declination
	gmt basemap -R-10/-2/12.8812380332/0.661018975345+r -JOc0/0/50/60/7i -B -BWSne -X1.25i --MAP_ANNOT_OBLIQUE=34 --FONT_ANNOT_PRIMARY=12p
	gmt basemap -Tmg-2/0.5+w2.5i+d-14.5+t45/10/5+i0.25p,blue+p0.25p,red+l+jCM \
		--FONT_ANNOT_PRIMARY=9p,Helvetica,blue --FONT_ANNOT_SECONDARY=12p,Helvetica,red --FONT_LABEL=14p,Times-Italic,darkgreen --FONT_TITLE=24p \
		--MAP_TITLE_OFFSET=7p --MAP_FRAME_WIDTH=10p --COLOR_BACKGROUND=green --MAP_DEFAULT_PEN=2p,darkgreen --COLOR_BACKGROUND=darkgreen \
		--MAP_VECTOR_SHAPE=0.5 --MAP_TICK_PEN_SECONDARY=thinner,red --MAP_TICK_PEN_PRIMARY=thinner,blue
	gmt inset begin -DjTR+w2.9i/3.9i+o0.05i -F+p+ggray95
	gmt inset end
	echo "5.5 3.8 GMT DEFAULTS" | gmt text -R0/7/0/4 -Jx1i -F+f14p,Helvetica-Bold+jCM
	gmt text -F+f11.5p+jLM << EOF
4.1 3.50 FONT_TITLE
4.1 3.25 MAP_TITLE_OFFSET
4.1 3.00 MAP_DEGREE_SYMBOL
4.1 2.75 @;blue;FONT_ANNOT_PRIMARY@;;
4.1 2.50 @;blue;MAP_TICK_PEN_PRIMARY@;;
4.1 2.25 @;blue;MAP_ANNOT_OFFSET_PRIMARY@;;
4.1 2.00 @;blue;MAP_TICK_LENGTH_PRIMARY@;;
4.1 1.75 @;red;FONT_ANNOT_SECONDARY@;;
4.1 1.50 @;red;MAP_TICK_PEN_SECONDARY@;;
4.1 1.25 @;red;MAP_ANNOT_OFFSET_SECONDARY@;;
4.1 1.00 @;red;MAP_TICK_LENGTH_SECONDARY@;;
4.1 0.75 @;darkgreen;FONT_LABEL@;;
4.1 0.50 @;darkgreen;MAP_DEFAULT_PEN@;;
4.1 0.25 @;darkgreen;COLOR_BACKGROUND@;;
EOF
gmt end show
