#!/bin/sh
#	$Id$
# Testing spectrum1d power spectrum values

ps=power1D.ps
# Single sinusoid of unit amplitude
gmtmath -T0/10239/1 T 10240 DIV 360 MUL 400 MUL COSD = t.txt
psxy -R4992/5504/-1.25/1.25 t.txt -JX6i/2i -B64f32:."Sinusoid @~l@~ = 25.6 from 0 to 10239":WSne -W2p -P -K -Xc > $ps
echo "4992 -1.25 ..." | pstext -R -J -O -K -F+jRM+f12p -D-0.3i/-0.18i -N >> $ps
echo "5504 -1.25 ..." | pstext -R -J -O -K -F+jLM+f12p -D+0.3i/-0.18i -N >> $ps
spectrum1d t.txt -S256 -W --GMT_FFT=brenner -N+pow5.txt -i1
#cut -f2 t.txt | /Volumes/MacNutHD3/UH/RESEARCH/CVSPROJECTS/GMTdev/gmt4/bin/spectrum1d -S256 -W -Nv4

psxy -R2/256/1e-19/100 -JX-6il/6.5il -Ba2f3/a-2pWsNe -O -K -Y3i pow5.txt -W0.25p >> $ps
psxy -R -J -O -K pow5.txt -Sc0.04i -Gblack -Ey >> $ps
spectrum1d t.txt -S256 -W --GMT_FFT=brenner -N+pow5.txt -i1 -L
psxy -R -J -O -K pow5.txt -W0.5p,green >> $ps
psxy -R -J -O -K v4.xpower -W0.25p,red >> $ps
psxy -R -J -O -W0.25p,- << EOF >> $ps
25.6	1e-19
25.6	100
EOF
