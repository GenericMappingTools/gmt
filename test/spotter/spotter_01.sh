#!/bin/bash
#
#       $Id$

ps=spotter_01.ps

# Example 1 - Using gmt backtracker
#
# We will use gmt backtracker to test all four functions.  We will
# 1. Plot hotspot track from Loihi forwards for 80 m.y.
# 2. forthtrack where Loihi will be in 80 m.y
# 3. Plot flowline from Suiko back until paleoridge (100 Ma)
# 4. Backtrack the location of Suiko using an age of 64.7 Ma

echo "205 20 80.0" > loihi.txt
echo "170 44 100" > suiko.txt
gmt pscoast -R150/220/00/65 -JM6i -P -K -G30/120/30 -A500 -Dl -W0.25p -B20 -BWSne > $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred -W0.5p loihi.txt >> $ps
# Task 1.1:
gmt backtracker loihi.txt -Df -Lb25 -E@WK97.txt | gmt psxy -R -J -O -K -W1p >> $ps
# Task 1.2:
gmt backtracker loihi.txt -Df -E@WK97.txt | gmt psxy -R -J -O -K -Sc0.1i -Ggreen -W0.5p >> $ps
# Task 1.3:
gmt backtracker suiko.txt -Db -Lf25 -E@WK97.txt | gmt psxy -R -JM -O -K -W1p,. >> $ps
echo "170 44 64.7" > suiko.txt
# Task 1.4:
gmt backtracker suiko.txt -Db -E@WK97.txt | gmt psxy -R -JM -O -K -St0.1i -Gyellow -W0.5p >> $ps
gmt psxy -R -JM -O -ST0.1i -Gcyan -W0.5p suiko.txt >> $ps

