#!/usr/bin/env bash
#

ps=spotter_12.ps

# Example 12 - Using gmt backtracker with drift (-F)
#
# We will use gmt backtracker to test all six functions with drift.  We will
# Go backwards in time from present coordinates to past coordinates:
#   1. Predict hotspot track from Loihi back to 80 m.y. [-Df -Lb]
#   2. Predict present location of point formed at Loihi 80 m.y ago [-Df]
#   3. Predict plate flowline through Suiko starting at present ridge [-Df -Lf]
#   7. Predict hotspot track from Loihi back to 80 m.y. if there were no drift [-Df -Lb]
# Go forward in time from the past to the present:
#   4. Predict plate flowline from Suiko back until paleoridge (80 Ma) [-Db -Lf]
#   5. Predict backtracked location of Suiko using its age of 64.7 Ma [-Db]
#   6. Predict hotspot track backwards from 80 m.y. to now [-Db -Lb]
#   8. Predict plate flowline back from 80 Ma point  [-Db -Lf]
gmt makecpt -Cjet -T0/80 > drift.cpt
echo "-155.2872 19.3972 80" > loihi.txt
echo "170 44 80" > suiko.txt
gmt backtracker loihi.txt -Df -ED2012x.txt -FD2012_HI_drift.txt > end_of_trail.txt
gmt backtracker suiko.txt -Db -ED2012x.txt > start_at_ridge.txt
gmt backtracker loihi.txt -Df -ED2012x.txt > end_of_flow.txt
#
gmt pscoast -R150/220/00/65 -JM7i -P -K -G30/120/30 -A500 -Dl -W0.25p -B20 -BWSne -Xc > $ps
gmt psxy -R -J -O -K -Sc0.1i -Cdrift.cpt D2012_HI_drift.txt >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gred -W0.5p loihi.txt >> $ps
# Task 1: [OK]
gmt backtracker loihi.txt -Df -Lb1 -ED2012x.txt -FD2012_HI_drift.txt | gmt psxy -R -J -O -K -W1p >> $ps
# Task 6: NO - gives flowline of tries with -Db -Lb since drift applied at wrong location.  Must do this instead:
gmt backtracker end_of_trail.txt -Db -ED2012x.txt -FD2012_HI_drift.txt > start_of_trail.txt
gmt backtracker start_of_trail.txt -Df -Lb5 -ED2012x.txt -FD2012_HI_drift.txt | gmt psxy -R -J -O -K -Sc0.2c -Wfaint >> $ps
# Task 2: [OK]
gmt backtracker loihi.txt -Df -ED2012x.txt -FD2012_HI_drift.txt | gmt psxy -R -J -O -K -Sc0.1i -Ggreen -W0.5p >> $ps
# Task 4: [OK]
gmt backtracker suiko.txt -Db -Lf1 -ED2012x.txt -FD2012_HI_drift.txt | gmt psxy -R -JM -O -K -W1p,orange >> $ps
# Task 3: [OK]
gmt backtracker start_at_ridge.txt -Df -Lf200 -ED2012x.txt -FD2012_HI_drift.txt | gmt psxy -R -JM -O -K -Sc0.2c -Wfaint,orange >> $ps
# Task 5: [OK]
gmt backtracker suiko.txt -Db -ED2012x.txt -FD2012_HI_drift.txt | gmt psxy -R -JM -O -K -Sc0.1c -Gblack >> $ps
echo "170 44 64.7" > suiko.txt
gmt backtracker suiko.txt -Db -ED2012x.txt -FD2012_HI_drift.txt | gmt psxy -R -JM -O -K -ST0.1i -Gyellow -W0.5p >> $ps
gmt psxy -R -JM -O -K -ST0.1i -Gcyan -W0.5p suiko.txt >> $ps
# Task 7: [OK]
gmt backtracker loihi.txt -Df -Lb -ED2012x.txt | gmt psxy -R -J -O -K -W0.5p,- >> $ps
# Task 8: [OK]
gmt backtracker end_of_flow.txt -Db -Lf200 -ED2012x.txt | gmt psxy -R -J -O -W0.5p,. >> $ps
