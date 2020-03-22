#!/usr/bin/env bash
# Addresses #message-6444 (no crossing near pole where longitudes change quickly).
# Solution is to process lon-array to avoid jumps.  This requires -fg since we cannot
# know if x is lon otherwise.  If range is 360 it may still fail; we issue a warning if so.
ps=lonjump.ps
# L1 has longitude jump across dateline but no -fg was given: No green circle
gmt psxy L1.txt -R-1/1.5/150/152+uk -JS0/90/6i -P -BWSne -Bafg1m -W0.5p,red -K -Xc > $ps
gmt psxy L1.txt -R -J -O -K -Sc0.2c -Gred >> $ps
gmt psxy L2.txt -R -J -O -K -W0.5p,blue >> $ps
gmt psxy L2.txt -R -J -O -K -Sc0.2c -Gblue >> $ps
gmt spatial L1.txt L2.txt -Ie -Fl | gmt psxy -R -J -O -K -Sc0.3c -Ggreen >> $ps
# Using -fg adjusts longitudes to have no jump: Green circle appears
gmt psxy L1.txt -R -J -O -Bafg1m -BWsNe -W0.5p,red -K -Y5i >> $ps
gmt psxy L1.txt -R -J -O -K -Sc0.2c -Gred >> $ps
gmt psxy L2.txt -R -J -O -K -W0.5p,blue >> $ps
gmt psxy L2.txt -R -J -O -K -Sc0.2c -Gblue >> $ps
gmt spatial L1.txt L2.txt -Ie -Fl -fg | gmt psxy -R -J -O -Sc0.5c -W0.5p >> $ps
