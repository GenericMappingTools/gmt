#!/bin/bash
# Test script for -p perspective. Ancient #131 issue.
# The p<az>/90 is wrongly positioned while th p180/90 is always correct.

ps=poption.ps
gmt psbasemap -Vd -R0/360/-90/0 -JA0/-90/4c -Bg360a360f360 -Xc0c -Yc0c -P -K -p45/90+w0/-90 > $ps
gmt psbasemap -Vd -R -J -P -O -Bg360a360f360 -p180/90+w0/-90 --MAP_GRID_PEN_PRIMARY=thin,blue --MAP_FRAME_PEN=thin,blue --MAP_TICK_PEN_PRIMARY=thin,blue >> $ps
