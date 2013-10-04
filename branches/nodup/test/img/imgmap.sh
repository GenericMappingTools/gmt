#!/bin/bash
#
#	$Id$

ps=imgmap.ps
IMG=topo.15.1.img
OK=`gmt gmtwhich -C $IMG`
if [ $OK = N ]; then
	echo "File $IMG not available for testing imgmap.sh"
	exit
fi
# Get merc grid
gmt gmt img2grd $IMG -R180/200/-5/5 -I1m -T1 -D -S1 -Gimg.nc -M
gmt gmt makecpt -Crainbow -T-8000/0/500 -Z > t.cpt
gmt gmt grdimage img.nc -Jx0.25i -Ct.cpt -P -K -Xc > $ps
gmt gmt psbasemap -R -Jm0.25i -Ba -BWSne -O -K >> $ps
# Get geo grid
gmt gmt img2grd $IMG -R -I1m -T1 -D -S1 -Gimg.nc
gmt gmt grdimage img.nc -Jm -Ct.cpt -O -K -Ba -BWSne -Y3.25i >> $ps
# Get resampled geo grid
gmt gmt img2grd $IMG -R -I1m -T1 -D -S1 -Gimg.nc -E
gmt gmt grdimage img.nc -Jm -Ct.cpt -O -K -Ba -BWSne -Y3.25i >> $ps
gmt gmt psxy -R -J -O -T >> $ps
