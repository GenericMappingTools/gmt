#!/bin/bash
#
#	$Id$
#
#	Examples of how to use the SPOTTER package
#
#	Paul Wessel
#	29-OCT-2015
#

POLES=WK97.txt	# Rotation poles to use
#POLES=DC85.txt	# Rotation poles to use

# Example 1 - Using gmt backtracker
#
# We will use gmt backtracker to test all four functions.  We will
# 1. Plot hotspot track from Loihi forwards for 80 m.y.
# 2. forthtrack where Loihi will be in 80 m.y
# 3. Plot flowline from Suiko back until paleoridge (100 Ma)
# 4. Backtrack the location of Suiko using an age of 64.7 Ma

echo -n "Running example 1..."

echo "205 20 80.0" > loihi.txt
echo "170 44 100" > suiko.txt
gmt pscoast -R150/220/00/65 -JM6i -P -K -darkgreen -A500 -Dl -W0.25p -Baf -BWSne > example_1.ps
gmt psxy -R -J -O -K -SC0.1i -Gred -W0.5p loihi.txt >> example_1.ps
# Task 1.1:
gmt backtracker loihi.txt -Df -Lb25 -E${POLES} | gmt psxy -R -J -O -K -M -W1p >> example_1.ps
# Task 1.2:
gmt backtracker loihi.txt -Df -E${POLES} | gmt psxy -R -J -O -K -SC0.1i -Ggreen -W0.5p >> example_1.ps
# Task 1.3:
gmt backtracker suiko.txt -Db -Lf25 -E${POLES} | gmt psxy -R -J -O -K -W1p,. >> example_1.ps
echo "170 44 64.7" > suiko.txt
# Task 1.4:
gmt backtracker suiko.txt -Db -E${POLES} | gmt psxy -R -J -O -K -ST0.1i -Gyellow -W0.5p >> example_1.ps
gmt psxy -R -J -O -ST0.1i -Gcyan -W0.5p suiko.txt >> example_1.ps
echo "Done.  View example_1.ps"
#gv example_1.ps

# Example 2 - Using gmt hotspotter
#
# We will use gmt hotspotter to create a CVA image for the Pacific.
# It will look similar to the ones we have published but we will
# here use only seamounts with a VGG amplitude of at least 100 Eotvos.

echo "Running example 2..."

DATA=seamounts.txt		# The data to use
tmax=145			# Upper age limit
dx=10m				# The grid spacing to use
region=130/260/-66/60		# Our Pacific region

gmt hotspotter $DATA -h -I$dx -R$region -E${POLES} -Gexample_1.nc -V -T -N$tmax

# Make a suitable color table

gmt makecpt -Chot -T0/3000 -Z > t.cpt

gmt grdimage example_1.nc -JM6i -P -K -Ct.cpt -V > example_2.ps
gmt pscoast -R -J -O -Gdarkgreen -A500 -Dl -W0.25p -Baf -BWSne >> example_2.ps
rm -f t.cpt loihi.txt suiko.txt
echo "Done.  View example_2.ps"
#gv example_2.ps

# Example 3 - Using gmt originater
#
# We will use gmt originater to determine the most likely hotspot origins
# for the seamounts in the seamounts.txt file, given a plate motion model
# and a list of possible hotspots.

echo "Running example 3..."

DATA=seamounts.txt		# The data to use
HS=pac_hs.txt			# The allowable hotspots to compare to
dx=10m				# The flowline sampling interval to use
region=130/260/-66/60		# Our Pacific region
N=2				# return the two most likely hotspots per seamount

gmt originater $DATA -S${N} -h -D$dx -E${POLES} -F${HS} -V > example_3.txt

echo "Done.  Inspect example_3.txt data file"
#$EDITOR example_3.txt
