#!/bin/sh
#       $Id: filter.sh,v 1.1 2011-06-11 00:36:54 guru Exp $
# Testing filter1d

. ../functions.sh
header "Test filter1d with or without -E option"

ps=filter.ps
# Make some random noise with a data gap, then save this in CVS as otherwise it would differ each time.
#gmtmath -T-500/500/10 T POP 0 100 NRAND = | awk '{if ($1 < -100 || $1 > 50) print $0}' | gmtconvert -gx50 > noise.txt
psxy -R-500/500/-300/300 -JX6i/2i -P -T -K -Y8.5i -Xc > $ps
psxy -R -J -B100g50/100g50WSne -O noise.txt -W0.25p,red,. -K > tmp.ps
psxy -R -J -O noise.txt -Sc0.05i -Gred -K >> tmp.ps
cat tmp.ps >> $ps
# Median without -E
psxy -R -J -O -T -K -Y-2.5i >> $ps
cat tmp.ps >> $ps
filter1d -Fm100 -N0 noise.txt | psxy -R -J -O -K -W2p,blue >> $ps
# Median with -E
psxy -R -J -O -T -K -Y-2.5i >> $ps
cat tmp.ps >> $ps
filter1d -Fm100 -E -N0 noise.txt | psxy -R -J -O -K -W2p,blue >> $ps
# Median with -E then Gaussian
psxy -R -J -O -T -K -Y-2.5i >> $ps
cat tmp.ps >> $ps
filter1d -Fm100 -E -N0 noise.txt | filter1d -Fg100 -E -N0 | psxy -R -J -O -K -W2p,blue >> $ps
psxy -R -J -O -T >> $ps

pscmp
rm -f tmp.ps
