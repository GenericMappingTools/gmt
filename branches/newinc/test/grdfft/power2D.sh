#!/bin/bash
#	$Id$
# Testing gmt grdfft power spectrum values

ps=power2D.ps

# Single sinusoid of unit amplitude
gmt grdmath -R0/256/0/256 -I1 -r X 256 DIV 360 MUL 10 MUL COSD = t.nc
gmt makecpt -Cpolar -T-1/1/0.1 -Z > t.cpt
gmt grdimage t.nc -JX6i -Ct.cpt -B64f32 -BWSne+t"Sinusoid @~l@~ = 25.6" -P -K -Xc > $ps

gmt grdfft t.nc -Exw -N+l --GMT_FFT=brenner > pow5.txt
# Compare with legacy result from GMT 4 (also Brenner FFT)
#/Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/gmt4/bin/gmt grdfft t.nc -Exw -Vl -L > pow4.txt

gmt psxy -R2/100/1e-19/1 -JX-6il/2.5il -Bxa2f3 -Bya-2p -BWsNe -O -K -Y7i pow5.txt -Sc0.15c -Ggreen >> $ps
gmt psxy -R -J -O -K pow4.txt -Sc0.075c -Gred >> $ps
gmt psxy -R -J -O -W0.25p,- << EOF >> $ps
>
25.6	1e-19
25.6	1
>
2	1e-15
100	1e-15
EOF
