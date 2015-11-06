#!/bin/bash
#
#	$Id$

GDAL=`gmt grdconvert 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi

ps=transparent_gif.ps

# Make several plots to test transparency
gmt psbasemap -R0/1/0/1 -JX7c -Y19c -B+glightblue+t"no option" -K -P > $ps
gmt psimage ${src:-.}/warning.gif -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -Y-9c -R -J -B+glightblue+t"-Gtblack" -O -K >> $ps
gmt psimage ${src:-.}/warning.gif -Gtblack -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -X8c -R -J -B+glightblue+t"-Gtwhite" -O -K >> $ps
gmt psimage ${src:-.}/warning.gif -Gtwhite -D0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -X-8c -Y-9c -R -J -B+glightblue+t"-Gtred" -O -K >> $ps
gmt psimage ${src:-.}/warning.gif -Gtred -Dx0.5c/0.5c+jBL+w6c -O -K >> $ps

gmt psbasemap -X8c -R -J -B+glightblue+t"-Gtblue" -O -K >> $ps
gmt psimage ${src:-.}/warning.gif -Gtblue -Dx0.5c/0.5c+jBL+w6c -O >> $ps
