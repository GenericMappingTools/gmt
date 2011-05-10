#!/bin/bash
#
#	$Id: readwrite_withgdal.sh,v 1.2 2011-05-10 03:06:48 guru Exp $

. ../functions.sh
if [ $GDAL -eq 0 ]; then exit; fi
	
header "Test grdimage for reading and writting images with GDAL"

ps=readwrite_withgdal.ps

# RGB image
grdimage -D needle.jpg -R1/430/1/300 -JX7c/0 -P -Y22c -K > $ps

# Same image as above but as idexed
grdimage -D needle.png -R1/430/1/300 -JX7c/0 -X7.5c -O -K >> $ps

# Projected
grdimage -Dr needle.jpg -Rd -JW12c -Bg30/g15 -X-6.0c -Y-7c -O -K >> $ps

# Illuminated
grdmath -R-15/15/-15/15 -I0.2 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL = somb.nc
grdgradient somb.nc -A225 -Gillum.nc -Nt0.75
grdimage -D needle.jpg -Iillum.nc -JM10c -Y-12c -O >> $ps

rm -f somb.nc illum.nc

pscmp
