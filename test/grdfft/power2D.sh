#!/bin/sh
#	$Id$
# Testing grdfft power spectrum values

ps=power2D.ps

# Single sinusoid of unit amplitude
grdmath -R0/256/0/256 -I1 -r X 256 DIV 360 MUL 10 MUL COSD = t.nc
makecpt -Cpolar -T-1/1/0.1 -Z > t.cpt
grdimage t.nc -JX6i -Ct.cpt -B64f32:."Sinusoid @~l@~ = 25.6":WSne -P -K -Xc > $ps

grdfft t.nc -Exw -N+l --GMT_FFT=brenner > pow5.txt
# Compare with legacy result from GMT 4 (also Brenner FFT)
#/Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/gmt4/bin/grdfft t.nc -Exw -Vl -L > pow4.txt

psxy -R2/100/1e-18/1 -JX-6il/2.5il -Ba2f3/a-2pWsNe -O -K -Y7i pow5.txt -W0.25p >> $ps
psxy -R -J -O -K pow4.txt -Sc0.1c -Gred >> $ps
psxy -R -J -O -W0.25p,- << EOF >> $ps
25.6	1e-18
25.6	1
EOF
