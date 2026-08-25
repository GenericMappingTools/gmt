#!/usr/bin/env bash
# Test trimming of lines in psxy via -W<pen>+o<offsets> for geo and Cartesian cases
ps=linetrim.ps
# Geographic
gmt psbasemap -R-5/100/-5/55 -JM6i -P -K -Baf -Xc > $ps
gmt math -T0/50/1 T = | gmt psxy -R -J -O -K -W2p >> $ps
gmt math -T0/50/1 T -C0 5 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 5 ADD = | gmt psxy -R -J -O -K -W2p+o1000k >> $ps
gmt math -T0/50/1 T -C0 10 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 10 ADD = | gmt psxy -R -J -O -K -W2p+o500k/1000k >> $ps
gmt math -T0/50/1 T -C0 15 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 15 ADD = | gmt psxy -R -J -O -K -W2p+o1i >> $ps
gmt math -T0/50/1 T -C0 20 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 20 ADD = | gmt psxy -R -J -O -K -W2p+o1i/0.5i >> $ps
gmt math -T0/50/1 T -C0 25 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 25 ADD = | gmt psxy -R -J -O -K -W2p+o1i/1000n >> $ps
gmt math -T0/50/1 T -C0 30 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 30 ADD = | gmt psxy -R -J -O -K -W2p+o1000k/0 >> $ps
gmt math -T0/50/1 T -C0 35 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 35 ADD = | gmt psxy -R -J -O -K -W2p+o0/1000k >> $ps
gmt math -T0/50/1 T -C0 40 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 40 ADD = | gmt psxy -R -J -O -K -W2p+o1i/0 >> $ps
gmt math -T0/50/1 T -C0 45 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 45 ADD = | gmt psxy -R -J -O -K -W2p+o0/1i >> $ps
# Cartesian
scl=$(gmt math -Q 6 105 DIV =)
gmt psbasemap -R -Jx${scl}i -O -K -Baf -Y5.5i >> $ps
gmt math -T0/50/1 T = | gmt psxy -R -J -O -K -W2p >> $ps
gmt math -T0/50/1 T -C0 5 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 5 ADD = | gmt psxy -R -J -O -K -W2p+o10 >> $ps
gmt math -T0/50/1 T -C0 10 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 10 ADD = | gmt psxy -R -J -O -K -W2p+o5/10 >> $ps
gmt math -T0/50/1 T -C0 15 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 15 ADD = | gmt psxy -R -J -O -K -W2p+o1i >> $ps
gmt math -T0/50/1 T -C0 20 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 20 ADD = | gmt psxy -R -J -O -K -W2p+o1i/0.5i >> $ps
gmt math -T0/50/1 T -C0 25 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 25 ADD = | gmt psxy -R -J -O -K -W2p+o1i/10 >> $ps
gmt math -T0/50/1 T -C0 30 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 30 ADD = | gmt psxy -R -J -O -K -W2p+o10/0 >> $ps
gmt math -T0/50/1 T -C0 35 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 35 ADD = | gmt psxy -R -J -O -K -W2p+o0/10 >> $ps
gmt math -T0/50/1 T -C0 40 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 40 ADD = | gmt psxy -R -J -O -K -W2p+o1i/0 >> $ps
gmt math -T0/50/1 T -C0 45 ADD = | gmt psxy -R -J -O -K -W2p+o0/1i >> $ps
gmt math -T0/50/1 T -C0 45 ADD = | gmt psxy -R -J -O -Wfaint,red >> $ps
