#!/usr/bin/env bash
# Plot the NASA Blue Marble 1x1 degree geotiff in different projections

ps=marbles.ps
gmt grdimage -JG0/0/3i @earth_day_01d -P -Y7i -K > $ps
gmt grdimage -JG90/30/3i @earth_day_01d -O -K -X3.5i >> $ps
gmt grdimage -JH270/6.5i @earth_day_01d -O -K -X-3.5i -Y-3.5i >> $ps
gmt grdimage -JM6.5i -R0/360/-45/45 -Bag @earth_day_01d -O -Y-2.5i >> $ps
