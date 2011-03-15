#!/bin/sh
#
#	$Id: spotter.sh,v 1.3 2011-03-15 02:06:37 guru Exp $
#
#	Examples of how to use the SPOTTER package
#
#	Paul Wessel
#	29-DEC-1999
#

POLES=WK97.d	# Rotation poles to use
#POLES=DC85.d	# Rotation poles to use

# Example 1 - Using backtracker
#
# We will use backtracker to test all four functions.  We will
# 1. Plot hotspot track from Loihi forwards for 80 m.y.
# 2. forthtrack where Loihi will be in 80 m.y
# 3. Plot flowline from Suiko back until paleoridge (100 Ma)
# 4. Backtrack the location of Suiko using an age of 64.7 Ma

echo -n "Running example 1..."

echo "205 20 80.0" > loihi.d
echo "170 44 100" > suiko.d
pscoast -R150/220/00/65 -JM6i -P -K -G30/120/30 -A500 -Dl -W0.25p -B20WSne > example_1.ps
psxy -R -JM -O -K -SC0.1 -G255/0/0 -W0.5p loihi.d >> example_1.ps
# Task 1.1:
backtracker loihi.d -Df -Lb25 -E${POLES} | psxy -R -JM -O -K -M -W1p >> example_1.ps
# Task 1.2:
backtracker loihi.d -Df -E${POLES} | psxy -R -JM -O -K -SC0.1 -G0/255/0 -W0.5p >> example_1.ps
# Task 1.3:
backtracker suiko.d -Db -Lf25 -E${POLES} | psxy -R -JM -O -K -M -W1top >> example_1.ps
echo "170 44 64.7" > suiko.d
# Task 1.4:
backtracker suiko.d -Db -E${POLES} | psxy -R -JM -O -K -ST0.1 -G255/255/0 -W0.5p >> example_1.ps
psxy -R -JM -O -ST0.1 -G0/255/255 -W0.5p suiko.d >> example_1.ps
echo "Done.  View example_1.ps"
#ghostview example_1.ps

# Example 2 - Using hotspotter
#
# We will use hotspotter to create a CVA image for the Pacific.
# It will look similar to the ones we have published but we will
# here use only seamounts with a VGG amplitude of at least 100 Eotvos.

echo "Running example 2..."

DATA=seamounts.d		# The data to use
tmax=145			# Upper age limit
dx=10m				# The grid spacing to use
region=130/260/-66/60		# Our Pacific region

hotspotter $DATA -h -I$dx -R$region -E${POLES} -Gexample_1.nc -V -T -N$tmax

# Make a suitable color table

makecpt -Chot -T0/3000/300 -Z > t.cpt

grdimage example_1.nc -JM6i -P -K -Ct.cpt -V > example_2.ps
pscoast -R -JM -O -G30/120/30 -A500 -Dl -W0.25p -B20WSne >> example_2.ps
\rm -f t.cpt loihi.d suiko.d
echo "Done.  View example_2.ps"
#ghostview example_2.ps

# Example 3 - Using originator
#
# We will use originator to determine the most likely hotspot origins
# for the seamounts in the seamounts.d file, given a plate motion model
# and a list of possible hotspots.

echo "Running example 3..."

DATA=seamounts.d		# The data to use
HS=pac_hs.d			# The allowable hotspots to compare to
dx=10m				# The flowline sampling interval to use
region=130/260/-66/60		# Our Pacific region
N=2				# return the two most likely hotspots per seamount

originator $DATA -S${N} -h -D$dx -E${POLES} -F${HS} -V > example_3.d

echo "Done.  Inspect example_3.d data file"
#$EDITOR example_3.d
