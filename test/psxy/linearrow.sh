#!/usr/bin/env bash
# Test adding arrows to lines in psxy via -W<pen>+o<offsets>+v<vecs> for geo and Cartesian cases
ps=linearrow.ps
# Geographic
gmt psbasemap -R-5/100/-5/55 -JM6i -P -K -Baf -Xc > $ps
gmt math -T0/50/1 T = | gmt psxy -R -J -O -K -W2p >> $ps
gmt math -T0/50/1 T -C0 5 ADD  = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 5 ADD  = | gmt psxy -R -J -O -K -W2p+o1000k+v0.15i+gblack >> $ps
gmt math -T0/50/1 T -C0 10 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 10 ADD = | gmt psxy -R -J -O -K -W2p+o1000k+v0.4i+p0.25p,blue+h1 >> $ps
gmt math -T0/50/1 T -C0 15 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 15 ADD = | gmt psxy -R -J -O -K -W2p+o1000k+v0.4i+p0.25p,blue+h0 >> $ps
gmt math -T0/50/1 T -C0 20 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 20 ADD = | gmt psxy -R -J -O -K -W2p+o1i/0.5i+ve0.3i+p1.5p,blue+gpink+ec+vb0.3i+p1.5p,blue+ggreen+es >> $ps
gmt math -T0/50/1 T -C0 25 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 25 ADD = | gmt psxy -R -J -O -K -W2p+o1i/1000n+v0.3i+gred >> $ps
gmt math -T0/50/1 T -C0 30 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 30 ADD = | gmt psxy -R -J -O -K -W2p+o1000k/0+ve0.3i+gblue >> $ps
gmt math -T0/50/1 T -C0 35 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 35 ADD = | gmt psxy -R -J -O -K -W2p+o0/1000k+ve0.3i+p1.5p,blue+et >> $ps
gmt math -T0/50/1 T -C0 40 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 40 ADD = | gmt psxy -R -J -O -K -W2p+o1i/0+v0.3i+p0.25p,blue+gcyan+h1 >> $ps
gmt math -T0/50/1 T -C0 45 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 45 ADD = | gmt psxy -R -J -O -K -W2p+o0/1i+vb0.3i+gblack+h0.5 >> $ps
# Cartesian
scl=$(gmt math -Q 6 105 DIV =)
gmt psbasemap -R -Jx${scl}i -O -K -Baf -Y5.5i >> $ps
gmt math -T0/50/1 T = | gmt psxy -R -J -O -K -W2p >> $ps
gmt math -T0/50/1 T -C0 5 ADD  = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 5 ADD  = | gmt psxy -R -J -O -K -W2p+o10+v0.15i+gblack >> $ps
gmt math -T0/50/1 T -C0 10 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 10 ADD = | gmt psxy -R -J -O -K -W2p+o10+v0.4i+p0.25p,blue+h1 >> $ps
gmt math -T0/50/1 T -C0 15 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 15 ADD = | gmt psxy -R -J -O -K -W2p+o10+v0.4i+p0.25p,blue+h0 >> $ps
gmt math -T0/50/1 T -C0 20 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 20 ADD = | gmt psxy -R -J -O -K -W2p+o1i/0.5i+ve0.3i+p1.5p,blue+gpink+ec+vb0.3i+p1.5p,blue+ggreen+es >> $ps
gmt math -T0/50/1 T -C0 25 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 25 ADD = | gmt psxy -R -J -O -K -W2p+o1i/10+v0.3i+gred >> $ps
gmt math -T0/50/1 T -C0 30 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 30 ADD = | gmt psxy -R -J -O -K -W2p+o10/0+ve0.3i+gblue >> $ps
gmt math -T0/50/1 T -C0 35 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 35 ADD = | gmt psxy -R -J -O -K -W2p+o0/10+ve0.3i+p1.5p,blue+et >> $ps
gmt math -T0/50/1 T -C0 40 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 40 ADD = | gmt psxy -R -J -O -K -W2p+o1i/0+v0.3i+p0.25p,blue+gcyan+h1 >> $ps
gmt math -T0/50/1 T -C0 45 ADD = | gmt psxy -R -J -O -K -Wfaint,red >> $ps
gmt math -T0/50/1 T -C0 45 ADD = | gmt psxy -R -J -O -W2p+o0/1i+vb0.3i+gblack+h0.5 >> $ps
