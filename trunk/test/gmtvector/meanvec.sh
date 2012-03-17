#!/bin/bash
# Test the gmtvector application


ps=meanvec.ps

psxy -R58/70/-68:30/-61 -JM6i -BafWSne -Sc0.3c -Gred -W0.25p pts.txt -P -K -Xc> $ps
gmtvector pts.txt -Am95 -fg > mean.txt
psxy -R -J -O -K -Sa0.5i -Gyellow -W1p mean.txt >> $ps
psxy -R -J -O -K -SE -W2p mean.txt >> $ps
gmtvector pts.txt -Am99 -fg | psxy -R -J -O -K -SE -W1p,- >> $ps
gmtvector pts.txt -Am90 -fg | psxy -R -J -O -K -SE -W1p,. >> $ps
echo 58 -68:30 90%, 95% and 99% confidence ellipses on mean position | pstext -R -J -O -K -F+jLB+f14p -T -W0.5p -Dj0.2i >> $ps
psxy -R -J -O -T >> $ps

