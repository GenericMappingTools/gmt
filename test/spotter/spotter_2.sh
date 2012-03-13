#!/bin/bash
#
#       $Id$

. ./functions.sh
header "Testing hotspotter to build CVA grids"

# Example 2 - Using hotspotter
#
# We will use hotspotter to create a CVA image for the Pacific.
# It will look similar to the ones we have published but we will
# here use only seamounts with a VGG amplitude of at least 100 Eotvos.

POLES="$src"/WK97.d			# Rotation poles to use

hotspotter "$src"/seamounts.d -I10m -R130/260/-66/60 -E${POLES} -Gspotter_2.nc -T -N145

# Make a suitable color table

makecpt -Chot -T0/3000/300 -Z > t.cpt

grdimage spotter_2.nc -JM6i -P -K -Ct.cpt > $ps
pscoast -R -J -O -Gdarkgreen -A500 -Dl -W0.25p -B20WSne >> $ps

pscmp
