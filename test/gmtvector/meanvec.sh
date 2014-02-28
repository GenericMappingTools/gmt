#!/bin/bash
# Test the gmt gmtvector application


ps=meanvec.ps

gmt psxy -R58/70/-68:30/-61 -JM6i -Baf -BWSne -Sc0.3c -Gred -W0.25p pts.txt -P -K -Xc> $ps
gmt gmtvector pts.txt -Am95 -fg > mean.txt
gmt psxy -R -J -O -K -Sa0.5i -Gyellow -W1p mean.txt >> $ps
gmt psxy -R -J -O -K -SE -W2p mean.txt >> $ps
gmt gmtvector pts.txt -Am99 -fg | gmt psxy -R -J -O -K -SE -W1p,- >> $ps
gmt gmtvector pts.txt -Am90 -fg | gmt psxy -R -J -O -K -SE -W1p,. >> $ps
echo 58 -68:30 90%, 95% and 99% confidence ellipses on mean position | gmt pstext -R -J -O -K -F+jLB+f14p -T -W0.5p -Dj0.2i >> $ps
gmt psxy -R -J -O -T >> $ps

