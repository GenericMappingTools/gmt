#!/bin/bash
ps=spotter_09.ps
# Create a small circle
gmt project -C0/-30 -E0/30  -Q -G500/60 > FZ.txt
# Make some "abyssal hills"
gmt grdmath -R-30/30/-35/35 -I1 -fg 0 = t.nc
gmt grdtrack -Gt.nc FZ.txt -C1000k/100k/100k+r -Ar > AH.txt
# write the bisectors to the FZs
gmt polespotter -FFZ.txt -D50 -L > g_FZ.txt
# Write the great circles parallel to AH
gmt polespotter -AAH.txt -D50 -L > g_AH.txt
gmt psxy -Rg -JQ0/7i -Baf -BWSne+t"Great circles from FZ and AH" -Wfaint,red g_FZ.txt -P -K > $ps
gmt psxy -R -J -O -K -Wfaint,blue g_AH.txt >> $ps
gmt psxy -R -J -O -K -W2p,red FZ.txt >> $ps
gmt psxy -R -J -O -K -W1p,blue AH.txt >> $ps
# Build density grid and solve for great-circle intersections
gmt polespotter -AAH.txt -FFZ.txt -D50 -Gpoles.nc -Rg -I1 -Cxings.txt
gmt makecpt -Cred,green,blue -T0,1,2,3 > a.cpt
gmt psxy -R -J -O -K -Sc0.01c -Ca.cpt -i0,1,4 xings.txt >> $ps
gmt makecpt -Chot -T0/25000 -I > t.cpt
gmt grdimage poles.nc -BWSne+t"Line density" -J -Ct.cpt -O  -Baf -Y5i >> $ps
