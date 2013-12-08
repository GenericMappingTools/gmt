#!/bin/bash
#
#	$Id$

GDAL=`gmt grdreformat 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi
	
ps=transparency.ps

# Make several plots to test transparency
gmt psbasemap -R0/1/0/1 -JX8c -B+gblue+t"no option" -K -P > $ps
gmt psimage warning.gif -C1c/1c/BL -W6c -O -K >> $ps

gmt psbasemap -X9c -R -J -B+gblue+t"-Gtwhite" -O -K >> $ps
gmt psimage warning.gif -Gtwhite -C1c/1c/BL -W6c -O -K >> $ps

gmt psbasemap -Y10c -R -J -B+gblue+t"-Gtred" -O -K >> $ps
gmt psimage warning.gif -Gtred -C1c/1c/BL -W6c -O -K >> $ps

gmt psbasemap -X-9c -R -J -B+gblue+t"-Gtblack" -O -K >> $ps
gmt psimage warning.gif -Gtblack -C1c/1c/BL -W6c -O >> $ps
