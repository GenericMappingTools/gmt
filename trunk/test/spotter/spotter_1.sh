#!/bin/bash
#
#       $Id: spotter_1.sh,v 1.3 2011-06-18 04:07:36 guru Exp $

. ../functions.sh
header "Testing backtracker fwd/back-ward"

ps=spotter_1.ps

POLES=WK97.d	# Rotation poles to use

# Example 1 - Using backtracker
#
# We will use backtracker to test all four functions.  We will
# 1. Plot hotspot track from Loihi forwards for 80 m.y.
# 2. forthtrack where Loihi will be in 80 m.y
# 3. Plot flowline from Suiko back until paleoridge (100 Ma)
# 4. Backtrack the location of Suiko using an age of 64.7 Ma

echo "205 20 80.0" > loihi.d
echo "170 44 100" > suiko.d
pscoast -R150/220/00/65 -JM6i -P -K -G30/120/30 -A500 -Dl -W0.25p -B20WSne > $ps
psxy -R -J -O -K -Sc0.1i -Gred -W0.5p loihi.d >> $ps
# Task 1.1:
backtracker loihi.d -Df -Lb25 -E${POLES} | psxy -R -J -O -K -W1p >> $ps
# Task 1.2:
backtracker loihi.d -Df -E${POLES} | psxy -R -J -O -K -Sc0.1i -Ggreen -W0.5p >> $ps
# Task 1.3:
backtracker suiko.d -Db -Lf25 -E${POLES} | psxy -R -JM -O -K -W1p,. >> $ps
echo "170 44 64.7" > suiko.d
# Task 1.4:
backtracker suiko.d -Db -E${POLES} | psxy -R -JM -O -K -St0.1i -Gyellow -W0.5p >> $ps
psxy -R -JM -O -ST0.1 -Gcyan -W0.5p suiko.d >> $ps
rm -f loihi.d  suiko.d

pscmp
