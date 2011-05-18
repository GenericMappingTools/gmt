#!/bin/bash
#
#	$Id: withgdal.sh,v 1.1 2011-05-18 00:40:06 jluis Exp $

. ../functions.sh
GDAL=`grdreformat 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi
	
header "Test psimage for reading images with GDAL"

ps=withgdal.ps

# RGB image
psimage ../grdimage/needle.jpg -W7c -P -Y15c -K > $ps

# Same image as above but as idexed
psimage ../grdimage/needle.png -W7c -X7.5c -O -K >> $ps

# Convert RGB to YIQ
psimage ../grdimage/needle.jpg -M -W7c -X-7.5c -Y-7c -O -K >> $ps

# Convert Indexed to YIQ
psimage ../grdimage/needle.png -M -W7c -X7.5c -O -K >> $ps

# A gray image (one band, no color map)
psimage ../grdimage/vader.jpg -W4c -X-2.5c -Y4.5c -O >> $ps

pscmp
