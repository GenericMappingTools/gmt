#!/usr/bin/env bash
# Test adding arrows to lines in psxy via -W<pen>+o<offsets>+v<vecs> for geo and Cartesian cases
ps=linearrow.ps
# Geographic
gmt psbasemap -R-5/100/-5/55 -JM6i -P -K -Baf -Xc > $ps
gmt math -T0/50/1 T = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 5 ADD  = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 5 ADD  = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1000k+v0.15i+gblack < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 10 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 10 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1000k+v0.4i+p0.25p,blue+h1 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 15 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 15 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1000k+v0.4i+p0.25p,blue+h0 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 20 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 20 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/0.5i+ve0.3i+p1.5p,blue+gpink+ec+vb0.3i+p1.5p,blue+ggreen+es < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 25 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 25 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/1000n+v0.3i+gred < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 30 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 30 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1000k/0+ve0.3i+gblue < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 35 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 35 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o0/1000k+ve0.3i+p1.5p,blue+et < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 40 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 40 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/0+v0.3i+p0.25p,blue+gcyan+h1 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 45 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 45 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o0/1i+vb0.3i+gblack+h0.5 < tmp_pipe.txt >> $ps
# Cartesian
scl=$(gmt math -Q 6 105 DIV =)
gmt psbasemap -R -Jx${scl}i -O -K -Baf -Y5.5i >> $ps
gmt math -T0/50/1 T = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 5 ADD  = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 5 ADD  = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o10+v0.15i+gblack < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 10 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 10 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o10+v0.4i+p0.25p,blue+h1 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 15 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 15 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o10+v0.4i+p0.25p,blue+h0 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 20 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 20 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/0.5i+ve0.3i+p1.5p,blue+gpink+ec+vb0.3i+p1.5p,blue+ggreen+es < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 25 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 25 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/10+v0.3i+gred < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 30 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 30 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o10/0+ve0.3i+gblue < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 35 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 35 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o0/10+ve0.3i+p1.5p,blue+et < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 40 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 40 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/0+v0.3i+p0.25p,blue+gcyan+h1 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 45 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 45 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -W2p+o0/1i+vb0.3i+gblack+h0.5 < tmp_pipe.txt >> $ps
