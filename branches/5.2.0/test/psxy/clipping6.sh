#!/bin/bash
ps=clipping6.ps
gmt psxy -R-90/270/-60/75 -JM5i -A -Ggreen -W0.25p,red badpol.txt -Bag -K -P -Xc > $ps
gmt psxy -R-180/180/-60/75 -JR90/5i -A -Ggreen -W0.25p,red badpol.txt -Bag -O -K -Y3.5i >> $ps
gmt psxy -R -JR-90/5i -A -Ggreen -W0.25p,red badpol.txt -Bag -O -Y3.5i >> $ps
