#!/usr/bin/env bash
#

GDAL=$(gmt grdconvert 2>&1 | grep -c gd)
if [ $GDAL -eq 0 ]; then exit; fi

ps=readwrite_withgdal.ps

# RGB image. The +b0,1,2 also tests the bands request option
gmt grdimage @needle.jpg+b0,1,2 -JX7c/0 -P -Y20c -K > $ps

# Same image as above but as idexed
gmt grdimage @needle.png -JX7c/0 -X7.5c -O -K >> $ps

# Projected
gmt grdimage @needle.jpg -Rd -JW10c -Bxg30 -Byg15 -X-5.0c -Y-7c -O -K >> $ps

# Illuminated
gmt grdmath -R-15/15/-15/15 -I0.2 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL = somb.nc
gmt grdgradient somb.nc -A225 -Gillum.nc -Nt0.75
gmt grdimage @needle.jpg -Iillum.nc -JM8c -Y-10c -X-2c -O -K >> $ps

# A gray image (one band, no color map)
gmt grdimage @vader.jpg -JX4c/0 -X9c -Y5c -K -O >> $ps

# Create a .png from a dummy grid and import it
gmt grdmath -R-5/5/-5/5 -I1 X Y MUL = lixo.grd
gmt makecpt -T-25/25/1 > lixo.cpt
gmt grdimage lixo.grd -Alixo.png=PNG -JX4c -Clixo.cpt
gmt grdimage lixo.png -JX4c -Y-5c -O >> $ps

