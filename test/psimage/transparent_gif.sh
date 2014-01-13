#!/bin/bash
#
#	$Id$

GDAL=`gmt grdreformat 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi
	
ps=transparent_gif.ps

# Make several plots to test transparency
gmt psbasemap -R0/1/0/1 -JX7c -Y19c -B+glightblue+t"no option" -K -P > $ps
gmt psimage ${src:-.}/warning.gif -C0.5c/0.5c/BL -W6c -O -K >> $ps

gmt psbasemap -Y-9c -R -J -B+glightblue+t"-Gtblack" -O -K >> $ps
gmt psimage ${src:-.}/warning.gif -Gtblack -C0.5c/0.5c/BL -W6c -O -K >> $ps

gmt psbasemap -X8c -R -J -B+glightblue+t"-Gtwhite" -O -K >> $ps
gmt psimage ${src:-.}/warning.gif -Gtwhite -C0.5c/0.5c/BL -W6c -O -K >> $ps

gmt psbasemap -X-8c -Y-9c -R -J -B+glightblue+t"-Gtred" -O -K >> $ps
gmt psimage ${src:-.}/warning.gif -Gtred -C0.5c/0.5c/BL -W6c -O -K >> $ps

gmt psbasemap -X8c -R -J -B+glightblue+t"-Gtblue" -O -K >> $ps
gmt psimage ${src:-.}/warning.gif -Gtblue -C0.5c/0.5c/BL -W6c -O >> $ps
