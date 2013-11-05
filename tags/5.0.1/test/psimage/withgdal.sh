#!/bin/bash
#
#	$Id$

. functions.sh
GDAL=`grdreformat 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi
	
header "Test psimage for reading images with GDAL"

# RGB image
psimage "$src"/../grdimage/gdal/needle.jpg -W7c -P -Y15c -K > $ps

# Same image as above but as idexed
psimage "$src"/../grdimage/gdal/needle.png -W7c -X7.5c -O -K >> $ps

# Convert RGB to YIQ
psimage "$src"/../grdimage/gdal/needle.jpg -M -W7c -X-7.5c -Y-7c -O -K >> $ps

# Convert Indexed to YIQ
psimage "$src"/../grdimage/gdal/needle.png -M -W7c -X7.5c -O -K >> $ps

# A gray image (one band, no color map)
psimage "$src"/../grdimage/gdal/vader.jpg -W4c -X-2.5c -Y4.5c -O >> $ps

pscmp
