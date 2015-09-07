#!/bin/bash
# Test segmentizing option -F in psxy with Cartesian data
ps=cartsegmentize.ps

function plotpts
{	# Plots the two data tables and places given text
	gmt psxy -R -J -O -K -Sc0.2c -Ggreen -Wfaint t1.txt
	gmt psxy -R -J -O -K -Sc0.2c -Gblue -Wfaint t2.txt
	echo $* | gmt pstext -R -J -O -K -F+cTL+jTL+f12p -Dj0.05i
}
cat << EOF > t1.txt
10	10
48	15
28	20
>
40	40
30	5
5	15
EOF
cat << EOF > t2.txt
7	20
29	11
8	4
EOF
scl=0.06i
# Show the data and its natural connectivity
gmt psxy -R0/50/0/45 -Jx${scl} -P -Ba10 -BWSne -W0.25p,- t[12].txt -K > $ps
plotpts TWO DATA TABLES >> $ps
# Lines from dataset origin
gmt psxy -R -J -Ba10 -BwSnE -W0.25p,- t[12].txt -O -K -X3.25i >> $ps
gmt psxy -R -J -W1p t[12].txt -Fd -O -K >> $ps
plotpts DATASET ORIGIN >> $ps
# Lines from table origin
gmt psxy -R -Jx${scl} -Ba10 -BWSne -W0.25p,- t[12].txt -O -K -X-3.25i -Y3.15i >> $ps
gmt psxy -R -J -W1p t[12].txt -Ft -O -K >> $ps
plotpts TABLE ORIGIN >> $ps
# Lines from segment origin
gmt psxy -R -J -Ba10 -BwSnE -W0.25p,- t[12].txt -O -K -X3.25i >> $ps
gmt psxy -R -J -W1p t[12].txt -Fs -O -K >> $ps
plotpts SEGMENT ORIGIN >> $ps
# Lines from fixed origin
gmt psxy -R -J -Ba10 -BWSne  -W0.25p,- t[12].txt -O -K -X-3.25i -Y3.15i >> $ps
gmt psxy -R -J -W1p t[12].txt -F10/35 -O -K >> $ps
plotpts FIXED ORIGIN >> $ps
echo 10 35 | gmt psxy -R -J -O -K -Sa0.4c -Gred -Wfaint >> $ps
# Lines for network
gmt psxy -R -J -Ba10 -BwSnE -W0.25p,- t[12].txt -O -K -X3.25i >> $ps
gmt psxy -R -J -W1p t[12].txt -Fn -O -K >> $ps
plotpts NETWORK >> $ps
gmt psxy -R -J -O -T >> $ps
