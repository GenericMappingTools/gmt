#!/usr/bin/env bash
# Testing gmt grdfft power spectrum values [radial]

ps=bworthr.ps

F0=0.25
gmt math -T0/0.5/0.001 T $F0 DIV 12 POW 1 ADD INV 128 PI DIV DIV T MUL = curve.txt
L=$(gmt math -Q 128 INV PI MUL 2 DIV =)
# White noise of unit amplitude: Saved since otherwise it would change each time
# gmt grdmath -R0/256/0/256 -I1 -r 0 1 NRAND = @white_noise.nc
gmt grdfft @white_noise.nc -Er -N+a > bworth_r.txt
gmt grdfft @white_noise.nc -Er+n -N+a > bworth_rn.txt
gmt grdfft @white_noise.nc -Fr-/4/6 -Er -N+a > bworth_br.txt
gmt grdfft @white_noise.nc -Fr-/4/6 -Er+n -N+a > bworth_brn.txt
A=$(gmt math -Sf -o1 bworth_rn.txt MEAN =)
gmt math -T0/0.5/0.001 T $F0 DIV 12 POW 1 ADD INV $A MUL = curven.txt
gmt psbasemap -R0/0.5/0/0.0125 -JX6i/4i -Baf -BWSne -P -K -X1.25i > $ps
gmt psxy -R -J -O -K -W0.5p << EOF >> $ps
0	0
0.5	$L
EOF
gmt psxy -R -J -O -K bworth_r.txt -Sc0.1c -Gblue -Ey >> $ps
gmt psxy -R -J -O -K bworth_r.txt -Wfaint,blue >> $ps
gmt psxy -R -J -O -K bworth_br.txt -Sc0.1c -Ggreen -Ey >> $ps
gmt psxy -R -J -O -K bworth_br.txt -W0.25p >> $ps
gmt psxy -R -J -O -K curve.txt -W0.5p,red >> $ps
gmt psbasemap -R0/0.5/0/2.5e-05  -J -Baf -BWSne+t"Butterworth Filtering (f@-0@- = 0.25, N = 6)" -O -K -Y4.5i >> $ps
gmt psxy -R -J -O -K -W0.5p << EOF >> $ps
0	$A
0.5	$A
EOF
gmt psxy -R -J -O -K bworth_rn.txt -Sc0.1c -Gblue -Ey >> $ps
gmt psxy -R -J -O -K bworth_rn.txt -Wfaint,blue >> $ps
gmt psxy -R -J -O -K bworth_brn.txt -Sc0.1c -Ggreen -Ey >> $ps
gmt psxy -R -J -O -K bworth_brn.txt -W0.25p >> $ps
gmt psxy -R -J -O -K curven.txt -W0.5p,red >> $ps
gmt psxy -R -J -O -T >> $ps
