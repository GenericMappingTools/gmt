#!/bin/bash
# Plot the NASA Blue Marble 1x1 degree geotiff in different projections
ps=marbles.ps
gmt grdimage -JG0/0/3i -D ${src:-.}/BlueMarble_60m.tiff -P -Y7i -K > $ps
gmt grdimage -JG90/30/3i -D ${src:-.}/BlueMarble_60m.tiff -O -K -X3.5i >> $ps
gmt grdimage -JH270/6.5i -D ${src:-.}/BlueMarble_60m.tiff -O -K -X-3.5i -Y-3.5i >> $ps
gmt grdimage -JM6.5i -R0/360/-45/45 -D -Bag ${src:-.}/BlueMarble_60m.tiff -O -Y-2.5i >> $ps
