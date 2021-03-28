#!/usr/bin/env bash

# Testing gmt math for VPDF
ps=vonmises.ps
gmt math -T0/360/1 T 150 10 VPDF = | gmt psxy -R0/360/0/1.5 -JX6i/6i -Bxaf+l"Angles" -Byaf+l"PDF" -BWSne+t"Von Mises probability density function" -W0.25p,red -P -K > $ps
gmt math -T0/360/1 T 150 4 VPDF = | gmt psxy -R -J -W0.5p,cyan -O -K >> $ps
gmt math -T0/360/1 T 150 4 VPDF = | gmt psxy -R -J -W0.75p,green -O -K >> $ps
gmt math -T0/360/1 T 150 3 VPDF = | gmt psxy -R -J -W1p,blue -O -K >> $ps
gmt math -T0/360/1 T 150 2 VPDF = | gmt psxy -R -J -W1.5p,yellow -O -K >> $ps
gmt math -T0/360/1 T 150 1 VPDF = | gmt psxy -R -J -W2p,black -O >> $ps
