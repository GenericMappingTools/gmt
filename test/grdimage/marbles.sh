#!/bin/bash
# Plot the NASA Blue Marble 1x1 degree geotiff in different projections
ps=marbles.ps
grdimage -JG0/0/3i -D BlueMarble_60m.tiff -P -Y7i -K > $ps
grdimage -JG90/30/3i -D BlueMarble_60m.tiff -O -K -X3.5i >> $ps
grdimage -JH270/6.5i -D BlueMarble_60m.tiff -O -K -X-3.5i -Y-3.5i >> $ps
grdimage -JM6.5i -R0/360/-45/45 -D -Bag BlueMarble_60m.tiff -O -Y-2.5i >> $ps