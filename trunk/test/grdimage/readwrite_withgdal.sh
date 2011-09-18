#!/bin/bash
#
#	$Id$

. ../functions.sh
GDAL=`grdreformat 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi
	
header "Test grdimage with images through GDAL"

ps=readwrite_withgdal.ps

# RGB image
grdimage -D gdal/needle.jpg -JX7c/0 -P -Y20c -K > $ps

# Same image as above but as idexed
grdimage -D gdal/needle.png -JX7c/0 -X7.5c -O -K >> $ps

# Projected
grdimage -Dr gdal/needle.jpg -Rd -JW10c -Bg30/g15 -X-5.0c -Y-7c -O -K >> $ps

# Illuminated
grdmath -R-15/15/-15/15 -I0.2 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL = somb.nc
grdgradient somb.nc -A225 -Gillum.nc -Nt0.75
grdimage -D gdal/needle.jpg -Iillum.nc -JM8c -Y-10c -X-2c -O -K >> $ps

# A gray image (one band, no color map)
grdimage -D gdal/vader.jpg -JX4c/0 -X9c -Y5c -K -O >> $ps

# Create a .png from a dummy grid and import it
grdmath -R-5/5/-5/5 -I1 X Y MUL = lixo.grd
makecpt -T-25/25/1 > lixo.cpt
grdimage lixo.grd -Alixo.png=PNG -JX4c -Clixo.cpt
grdimage -D lixo.png -JX4c -Y-5c -O >> $ps

rm -f somb.nc illum.nc lixo.png lixo.cpt lixo.grd

pscmp
