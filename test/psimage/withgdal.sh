#!/bin/bash
#
#	$Id$

GDAL=`gmt grdconvert 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi
	
ps=withgdal.ps

# RGB image
gmt psimage "${src:-.}"/../grdimage/gdal/needle.jpg -Dx0/0+w7c -P -Y15c -K > $ps

# Same image as above but as idexed
gmt psimage "${src:-.}"/../grdimage/gdal/needle.png -Dx0/0+w7c -X7.5c -O -K >> $ps

# Convert RGB to YIQ
gmt psimage "${src:-.}"/../grdimage/gdal/needle.jpg -M -Dx0/0+w7c -X-7.5c -Y-7c -O -K >> $ps

# Convert Indexed to YIQ
gmt psimage "${src:-.}"/../grdimage/gdal/needle.png -M -Dx0/0+w7c -X7.5c -O -K >> $ps

# A gray image (one band, no color map)
gmt psimage "${src:-.}"/../grdimage/gdal/vader.jpg -Dx0/0+w4c -X-2.5c -Y4.5c -O >> $ps

