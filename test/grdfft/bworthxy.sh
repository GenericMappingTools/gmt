#!/usr/bin/env bash
# Testing gmt grdfft power spectrum values

ps=bworthxy.ps

F0=0.25
gmt math -T0/0.5/0.001 T $F0 DIV 12 POW 1 ADD INV 128 DIV = curve.txt
L=$(gmt math -Q 128 INV =)
# White noise of unit amplitude: Saved since otherwise it would change each time
# gmt grdmath -R0/256/0/256 -I1 -r 0 1 NRAND = @white_noise.nc
gmt grdfft @white_noise.nc -Ex -N+a > bworth_x.txt
gmt grdfft @white_noise.nc -Ey -N+a > bworth_y.txt
gmt grdfft @white_noise.nc -Fx-/4/6 -Ex -N+a > bworth_bx.txt
gmt grdfft @white_noise.nc -Fy-/4/6 -Ey -N+a > bworth_by.txt

gmt psbasemap -R0/0.5/0/0.0125 -JX6i/4i -Baf -BWSne -P -K -X1.25i > $ps
gmt psxy -R -J -O -K -W0.5p << EOF >> $ps
0	$L
0.5	$L
EOF
gmt psxy -R -J -O -K bworth_x.txt -Sc0.1c -Gblue -Ey >> $ps
gmt psxy -R -J -O -K bworth_x.txt -Wfaint,blue >> $ps
gmt psxy -R -J -O -K bworth_bx.txt -Sc0.1c -Ggreen -Ey >> $ps
gmt psxy -R -J -O -K bworth_bx.txt -W0.25p >> $ps
gmt psxy -R -J -O -K curve.txt -W0.5p,red >> $ps
gmt psbasemap -R -J -Baf -BWSne+t"Butterworth Filtering (f@-0@- = 0.25, N = 6)" -O -K -Y4.5i >> $ps
gmt psxy -R -J -O -K -W0.5p << EOF >> $ps
0	$L
0.5	$L
EOF
gmt psxy -R -J -O -K bworth_y.txt -Sc0.1c -Gblue -Ey >> $ps
gmt psxy -R -J -O -K bworth_y.txt -Wfaint,blue >> $ps
gmt psxy -R -J -O -K bworth_by.txt -Sc0.1c -Ggreen -Ey >> $ps
gmt psxy -R -J -O -K bworth_by.txt -W0.25p >> $ps
gmt psxy -R -J -O -K curve.txt -W0.5p,red >> $ps
gmt psxy -R -J -O -T >> $ps
