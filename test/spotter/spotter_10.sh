#!/usr/bin/env bash
ps=spotter_10.ps
# Create a small circle
gmt project -C0/-30 -E0/30  -Q -G500/60 > FZ.txt
# Make some "abyssal hills"
gmt grdmath -R-30/30/-35/35 -I1 -fg 0 = t.nc
gmt grdtrack -Gt.nc FZ.txt -C1000k/100k/100k+r -Ar > AH.txt
# Determine the pole chi-squared just from FZs
gmt polespotter -FFZ.txt -Sp -R-80/-40/-20/20 -I20m -Gmisfit.nc -N
gmt makecpt -Chot -T0/0.01 > t.cpt
gmt grdimage misfit.nc -BWSne -JM2.85i -Ct.cpt -P -K -Baf -X1.25i -Y0.75i > $ps
gmt grdcontour misfit.nc -J -O -K -C0.01 -L0.009/0.011 >> $ps
gmt pstext -R -J -O -K -F+f18p+cTL+jTL+tFZ -Dj0.2i -Gwhite -W1p >> $ps
# Determine the pole chi-squared just from AH
gmt polespotter -AAH.txt -Sp -R-80/-40/-20/20 -I20m -Gmisfit.nc -N
gmt grdimage misfit.nc -BwSne -J -Ct.cpt -O -K -Baf -X3.15i >> $ps
gmt grdcontour misfit.nc -J -O -K -C0.01 -L0.009/0.011 >> $ps
gmt pstext -R -J -O -K -F+f18p+cTL+jTL+tAH -Dj0.2i -Gwhite -W1p >> $ps
# Determine the pole chi-squared from both FZ AH
gmt polespotter -AAH.txt -FFZ.txt -Sp -R-80/-40/-20/20 -I20m -Gmisfit.nc -N
gmt grdimage misfit.nc -BWSne -JM6i -Ct.cpt -O -K -Baf -X-3.15i -Y3.35i >> $ps
gmt grdcontour misfit.nc -J -O -K -C0.01 -L0.009/0.011 >> $ps
gmt pstext -R -J -O -F+f18p+cTL+jTL+t"AH+FZ" -Dj0.2i -Gwhite -W1p >> $ps
