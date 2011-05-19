#!/bin/bash
#
#	$Id: readwrite_withgdal.sh,v 1.5 2011-05-19 17:11:52 remko Exp $

. ../functions.sh
GDAL=`grdreformat 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi
	
header "Test grdimage with images through GDAL"

ps=readwrite_withgdal.ps

# RGB image
grdimage -D needle.jpg -JX7c -P -Y20c -K > $ps

# Same image as above but as idexed
grdimage -D needle.png -JX7c -X7.5c -O -K >> $ps

# Projected
grdimage -Dr needle.jpg -Rd -JW10c -Bg30/g15 -X-5.0c -Y-7c -O -K >> $ps

# Illuminated
grdmath -R-15/15/-15/15 -I0.2 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL = somb.nc
grdgradient somb.nc -A225 -Gillum.nc -Nt0.75
grdimage -D needle.jpg -Iillum.nc -JM8c -Y-10c -X-2c -O -K >> $ps

# A gray image (one band, no color map)
grdimage -D vader.jpg -JX4c/0 -X9c -O >> $ps

rm -f somb.nc illum.nc

pscmp
