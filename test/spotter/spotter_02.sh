#!/bin/bash
#

ps=spotter_02.ps

# Example 2 - Using gmt hotspotter
#
# We will use gmt hotspotter to create a CVA image for the Pacific.
# It will look similar to the ones we have published but we will
# here use only seamounts with a VGG amplitude of at least 100 Eotvos.

gmt hotspotter @seamounts.txt -I10m -R130/260/-66/60 -E@WK97.txt -Gspotter_2.nc -T -N145

# Make a suitable color table

gmt makecpt -Chot -T0/3000 > t.cpt

gmt grdimage spotter_2.nc -JM6i -P -K -Ct.cpt > $ps
gmt pscoast -R -J -O -Gdarkgreen -A500 -Dl -W0.25p -B20 -BWSne >> $ps

