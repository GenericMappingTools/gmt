#!/usr/bin/env bash
# Test the gmt vector application


ps=meanvec.ps

gmt psxy -R58/70/-68:30/-61 -JM6i -Bafg1 -BWSne -Sc0.3c -Gred -W0.25p pts.txt -P -K -Xc > $ps
gmt vector pts.txt -Am95 -fg -E > mean.txt
gmt psxy -R -J -O -K -Sa0.5i -Gyellow -W1p mean.txt >> $ps
gmt psxy -R -J -O -K -SE -W2p mean.txt >> $ps
gmt vector pts.txt -Am99 -fg -E | gmt psxy -R58/70/-68:30/-61 -JM6i -O -K -SE -W1p,- >> $ps
gmt vector pts.txt -Am90 -fg -E | gmt psxy -R58/70/-68:30/-61 -JM6i -O -K -SE -W1p,. >> $ps
echo 58 -68:30 90%, 95% and 99% confidence ellipses on mean position | gmt pstext -R58/70/-68:30/-61 -JM6i -O -F+jLB+f14p -T -W0.5p -Dj0.2i -Gwhite >> $ps
