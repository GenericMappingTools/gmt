#!/usr/bin/env bash
# Test trimming of lines in psxy via -W<pen>+o<offsets> for geo and Cartesian cases
ps=linetrim.ps
# Geographic
gmt psbasemap -R-5/100/-5/55 -JM6i -P -K -Baf -Xc > $ps
gmt math -T0/50/1 T = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 5 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 5 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1000k < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 10 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 10 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o500k/1000k < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 15 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 15 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 20 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 20 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/0.5i < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 25 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 25 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/1000n < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 30 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 30 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1000k/0 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 35 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 35 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o0/1000k < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 40 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 40 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/0 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 45 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 45 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o0/1i < tmp_pipe.txt >> $ps
# Cartesian
scl=$(gmt math -Q 6 105 DIV =)
gmt psbasemap -R -Jx${scl}i -O -K -Baf -Y5.5i >> $ps
gmt math -T0/50/1 T = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 5 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 5 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o10 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 10 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 10 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o5/10 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 15 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 15 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 20 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 20 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/0.5i < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 25 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 25 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/10 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 30 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 30 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o10/0 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 35 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 35 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o0/10 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 40 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -Wfaint,red < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 40 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o1i/0 < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 45 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -K -W2p+o0/1i < tmp_pipe.txt >> $ps
gmt math -T0/50/1 T -C0 45 ADD = > tmp_pipe.txt
gmt psxy -R -J -O -Wfaint,red < tmp_pipe.txt >> $ps
