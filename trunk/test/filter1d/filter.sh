#!/bin/bash
#       $Id$
# Testing gmt filter1d

ps=filter.ps

# Make some random noise with a data gap, then save this in CVS as otherwise it would differ each time.
#gmt gmtmath -T-500/500/10 T POP 0 100 NRAND = | $AWK '{if ($1 < -100 || $1 > 50) print $0}' | gmt gmtconvert -gx50 > noise.txt
gmt psxy -R-500/500/-300/300 -JX6i/2i -P -T -K -Y8.5i -Xc > $ps
gmt psxy -R -J -Bx100g50 -By100g50 -BWSne -O noise.txt -W0.25p,red,. -K > tmp.eps
gmt psxy -R -J -O noise.txt -Sc0.05i -Gred -K >> tmp.eps
cat tmp.eps >> $ps
# Median without -E
gmt psxy -R -J -O -T -K -Y-2.5i >> $ps
cat tmp.eps >> $ps
gmt filter1d -Fm100 -N0 noise.txt | gmt psxy -R-500/500/-300/300 -J -O -K -W2p,blue >> $ps
# Median with -E
gmt psxy -R -J -O -T -K -Y-2.5i >> $ps
cat tmp.eps >> $ps
gmt filter1d -Fm100 -E -N0 noise.txt | gmt psxy -R-500/500/-300/300 -J -O -K -W2p,blue >> $ps
# Median with -E then Gaussian
gmt psxy -R -J -O -T -K -Y-2.5i >> $ps
cat tmp.eps >> $ps
gmt filter1d -Fm100 -E -N0 noise.txt | gmt filter1d -Fg100 -E -N0 | gmt psxy -R-500/500/-300/300 -J -O -K -W2p,blue >> $ps
gmt psxy -R -J -O -T >> $ps
