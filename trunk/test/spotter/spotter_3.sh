#!/bin/bash
#
#       $Id: spotter_3.sh,v 1.1 2011-06-14 01:28:40 guru Exp $

. ../functions.sh
header "Test spotter: originator"

ps=spotter_3.ps

# Example 3 - Using originator
#
# We will use originator to determine the most likely hotspot origins
# for the seamounts in the seamounts.d file, given a plate motion model
# and a list of possible hotspots.

DATA=seamounts.d		# The data to use
POLES=WK97.d			# Rotation poles to use
HS=pac_hs.d			# The allowable hotspots to compare to
dx=10m				# The flowline sampling interval to use
region=130/260/-66/60		# Our Pacific region
N=1				# return the two most likely hotspots per seamount

originator $DATA -S${N} -D$dx -E${POLES} -F${HS} > spotter_3.d

echo "Done.  Inspect spotter_3.d data file"
#pscmp
