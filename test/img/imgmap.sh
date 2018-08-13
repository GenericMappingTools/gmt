#!/bin/bash
#

ps=imgmap.ps
IMG=@topo.8.2.img
# Get merc grid
gmt img2grd $IMG -R180/200/-5/5 -T1 -S1 -Gimg.nc -M
gmt makecpt -Crainbow -T-8000/0 > t.cpt
gmt grdimage img.nc -Jx0.25i -Ct.cpt -P -K -Xc > $ps
gmt psbasemap -R -Jm0.25i -Ba -BWSne -O -K >> $ps
# Get geo grid
gmt img2grd $IMG -R -T1 -S1 -Gimg.nc
gmt grdimage img.nc -Jm -Ct.cpt -O -K -Ba -BWSne -Y3.25i >> $ps
# Get resampled geo grid
gmt img2grd $IMG -R -T1 -S1 -Gimg.nc -E
gmt grdimage img.nc -Jm -Ct.cpt -O -K -Ba -BWSne -Y3.25i >> $ps
gmt psxy -R -J -O -T >> $ps
