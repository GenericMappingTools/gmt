#!/bin/bash
#	$Id$
# Testing gmt spectrum1d power spectrum values

ps=power1D.ps
# Single sinusoid of unit amplitude
gmt gmtmath -T0/10239/1 T 10240 DIV 360 MUL 400 MUL COSD = t.txt
gmt psxy -R4992/5504/-1.25/1.25 t.txt -JX6i/2i -B64f32 -BWSne+t"Sinusoid @~l@~ = 25.6 from 0 to 10239" -W2p -P -K -Xc > $ps
echo "4992 -1.25 ..." | gmt pstext -R -J -O -K -F+jRM+f12p -D-0.3i/-0.18i -N >> $ps
echo "5504 -1.25 ..." | gmt pstext -R -J -O -K -F+jLM+f12p -D+0.3i/-0.18i -N >> $ps
gmt spectrum1d t.txt -S256 -W --GMT_FFT=brenner -N+pow5.txt -i1
#cut -f2 t.txt | /Volumes/MacNutHD3/UH/RESEARCH/CVSPROJECTS/GMTdev/gmt4/bin/gmt spectrum1d -S256 -W -Nv4

gmt psxy -R2/256/1e-19/100 -JX-6il/6.5il -Bxa2f3 -Bya-2p -BWsNe -O -K -Y3i pow5.txt -W0.25p >> $ps
gmt psxy -R -J -O -K pow5.txt -Sc0.04i -Gblack -Ey >> $ps
gmt spectrum1d t.txt -S256 -W --GMT_FFT=brenner -N+pow5.txt -i1 -L
gmt psxy -R -J -O -K pow5.txt -W0.5p,green >> $ps
gmt psxy -R -J -O -K v4.xpower -W0.25p,red >> $ps
gmt psxy -R -J -O -W0.25p,- << EOF >> $ps
25.6	1e-19
25.6	100
EOF
