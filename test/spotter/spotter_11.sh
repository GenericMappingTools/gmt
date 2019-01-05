#!/usr/bin/env bash
#

ps=spotter_11.ps

# Example 11 - Using gmt backtracker [repeat of 1 with a few more things]
#
# We will use gmt backtracker to test all six functions.  We will
# Go backwards in time from present coordinates to past coordinates:
#   1. Predict hotspot track from Loihi back to 80 m.y. [-Df -Lb]
#   2. Predict present location of point formed at Loihi 80 m.y ago [-Df]
#   3. Predict plate flowline through Suiko starting at present ridge [-Df -Lf]
# Go forward in time from the past to the present:
#   4. Predict plate flowline from Suiko back until paleoridge (100 Ma) [-Db -Lf]
#   5. Predict backtracked location of Suiko using its age of 64.7 Ma [-Db]
#   6. Predict hotspot track backwards from 80 m.y. to now [-Db -Lb]
echo "205 20  80" > loihi.txt
echo "170 44 100" > suiko.txt
gmt backtracker loihi.txt -Df -E@WK97.txt > end_of_trail.txt
gmt backtracker suiko.txt -Db -E@WK97.txt > start_at_ridge.txt
#
gmt pscoast -R150/220/00/65 -JM7i -P -K -G30/120/30 -A500 -Dl -W0.25p -B20 -BWSne -Xc > $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred -W0.5p loihi.txt >> $ps
# Task 1:
gmt backtracker loihi.txt -Df -Lb25 -E@WK97.txt | gmt psxy -R -J -O -K -W1p >> $ps
# Task 6:
gmt backtracker end_of_trail.txt -Db -Lb200 -E@WK97.txt | gmt psxy -R -J -O -K -Sc0.2c -Wfaint >> $ps
# Task 2:
gmt backtracker loihi.txt -Df -E@WK97.txt | gmt psxy -R -J -O -K -Sc0.1i -Ggreen -W0.5p >> $ps
# Task 4:
gmt backtracker suiko.txt -Db -Lf25 -E@WK97.txt | gmt psxy -R -JM -O -K -W1p,orange >> $ps
# Task 3:
gmt backtracker start_at_ridge.txt -Df -Lf200 -E@WK97.txt | gmt psxy -R -JM -O -K -Sc0.2c -Wfaint,orange >> $ps
# Task 5:
echo "170 44 64.7" > suiko.txt
gmt backtracker suiko.txt -Db -E@WK97.txt | gmt psxy -R -JM -O -K -ST0.1i -Gyellow -W0.5p >> $ps
gmt psxy -R -JM -O -ST0.1i -Gcyan -W0.5p suiko.txt >> $ps

