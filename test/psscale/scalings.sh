#!/usr/bin/env bash
# Addresses issue #1205.  The bottom plot is all auto and
# correct, while the top plot failed to flip the annotation
# side when +m was added, because it had already been flipped
# for BC and RM (the cause of the issue).  I know check if
# flipped and then +m implies the opposite flip.
ps=scalings.ps

gmt psbasemap -R0/10/0/10 -JX12c -B0 -K -P -Xc -Y1c > $ps

# plot color bar without +m option
gmt psscale -J -R -Cseis -DjTL+w3c/0.5c -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjML+w3c/0.5c -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjBL+w3c/0.5c -Ba -K -O >> $ps

gmt psscale -J -R -Cseis -DjTC+w3c/0.5c -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjMC+w3c/0.5c -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjBC+w3c/0.5c -Ba -K -O >> $ps

gmt psscale -J -R -Cseis -DjTR+w3c/0.5c -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjMR+w3c/0.5c -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjBR+w3c/0.5c -Ba -K -O >> $ps

gmt psbasemap -R -J -B0 -Y13c -K -O >> $ps

# plot color bar with +m option
gmt psscale -J -R -Cseis -DjTL+w3c/0.5c+m -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjML+w3c/0.5c+m -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjBL+w3c/0.5c+m -Ba -K -O >> $ps

gmt psscale -J -R -Cseis -DjTC+w3c/0.5c+m -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjMC+w3c/0.5c+m -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjBC+w3c/0.5c+m -Ba -K -O >> $ps

gmt psscale -J -R -Cseis -DjTR+w3c/0.5c+m -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjMR+w3c/0.5c+m -Ba -K -O >> $ps
gmt psscale -J -R -Cseis -DjBR+w3c/0.5c+m -Ba -O >> $ps
