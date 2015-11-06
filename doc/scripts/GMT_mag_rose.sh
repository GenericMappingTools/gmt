#!/bin/bash
#	$Id$
ps=GMT_mag_rose.ps
# Magnetic rose with a specified declination
gmt psbasemap -R-10/-2/12.8812380332/0.661018975345r -JOc0/0/50/60/7i -Baf -BWSne -P -K -X1.25i --MAP_ANNOT_OBLIQUE=34 --FONT_ANNOT_PRIMARY=12p > $ps
gmt psbasemap -R -J -Tmg-2/0.5+w2.5i+d-14.5+t45/10/5+i0.25p,blue+p0.25p,red+l+jCM -O -K \
	--FONT_ANNOT_PRIMARY=9p,Helvetica,blue --FONT_ANNOT_SECONDARY=12p,Helvetica,red --FONT_LABEL=14p,Times-Italic,darkgreen --FONT_TITLE=24p \
	--MAP_TITLE_OFFSET=7p --MAP_FRAME_WIDTH=10p --COLOR_BACKGROUND=green --MAP_DEFAULT_PEN=2p,darkgreen --COLOR_BACKGROUND=darkgreen \
	--MAP_VECTOR_SHAPE=0.5 --MAP_TICK_PEN_SECONDARY=thinner,red --MAP_TICK_PEN_PRIMARY=thinner,blue >> $ps
gmt psbasemap -R -J -O -K -DjTR+w2.9i/3.9i+o0.05i -F+p+ggray95 >> $ps
echo "5.5 3.8 GMT DEFAULTS" | gmt pstext -R0/7/0/5 -Jx1i -O -K -F+f14p,Helvetica-Bold+jCM >> $ps
gmt pstext -R -J -O -K -F+f12p+jLM << EOF >> $ps
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
gmt psxy -R -J -O -T >> $ps
