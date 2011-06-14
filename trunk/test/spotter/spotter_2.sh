#!/bin/bash
#
#       $Id: spotter_2.sh,v 1.1 2011-06-14 01:28:40 guru Exp $

. ../functions.sh
header "Test spotter: hotspotter"

ps=spotter_2.ps

# Example 2 - Using hotspotter
#
# We will use hotspotter to create a CVA image for the Pacific.
# It will look similar to the ones we have published but we will
# here use only seamounts with a VGG amplitude of at least 100 Eotvos.

DATA=seamounts.d		# The data to use
POLES=WK97.d			# Rotation poles to use
tmax=145			# Upper age limit
dx=10m				# The grid spacing to use
region=130/260/-66/60		# Our Pacific region

hotspotter $DATA -I$dx -R$region -E${POLES} -Gspotter_2.nc -T -N$tmax

# Make a suitable color table

makecpt -Chot -T0/3000/300 -Z > t.cpt

grdimage spotter_2.nc -JM6i -P -K -Ct.cpt > $ps
pscoast -R -J -O -G30/120/30 -A500 -Dl -W0.25p -B20WSne >> $ps
#rm -f t.cpt spotter_2.nc

pscmp
