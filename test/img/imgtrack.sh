#!/usr/bin/env bash
# Testing various ways of extracting and plotting IMG grids

ps=imgtrack.ps
IMG=@topo.8.4.img
# Sample an img file along a track directly
gmt makecpt -Crainbow -T-8000/0 > t.cpt
gmt grdtrack -G${IMG},1,1 -R180/200/40/50 -EBL/TR > m.txt
gmt psxy -R -JM6i -P -Baf -Sc0.1c -Ct.cpt m.txt -K > $ps
gmt img2grd $IMG -R -T1 -S1 -Gimg_g.nc
gmt grdtrack -Gimg_g.nc -EBL/TR > g.txt
gmt psxy -R -J -O -Baf -Sc0.1c -Ct.cpt g.txt -Y5i >> $ps
