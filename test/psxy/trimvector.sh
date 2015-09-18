#!/bin/bash
#	$Id$
#
# Test vectors with mid-heads and trimming

ps=trimvector.ps
function plot { # The args are -X -Y and -Sv<something>
	gmt psxy -R0/1.75/0/1.8 -J -O -K v.txt -Gred -W0.5p -B0 $*
	gmt psxy -R -J -O -K p.txt -Sc0.2i -Wfaint
}

cat << EOF >p.txt
0.3	0.3
1.5	1.0
0.7	1.6
EOF
gmt convert p.txt -Fv p.txt > v.txt
gmt set MAP_VECTOR_SHAPE 0.5
gmt psxy -R0/7/0/9 -Jx1i -P -K -T -Xc > $ps
# Vectors at various ends and one with no vector
plot -Sv0.2i+t0.1i+s+e >> $ps
plot -Sv0.2i+t0.1i+s+b -X1.75i >> $ps
plot -Sv0.2i+t0.1i+s+b+e -X1.75i >> $ps
plot -Sv0.2i+t0.1i+s -X1.75i >> $ps
# Mid-point vectors and half vectors
plot -Sv0.2i+t0.1i+s+mf -X-5.25i -Y1.8i >> $ps
plot -Sv0.2i+t0.1i+s+mr -X1.75i >> $ps
plot -Sv0.2i+t0.1i+s+mfl -X1.75i >> $ps
plot -Sv0.2i+t0.1i+s+mfr -X1.75i >> $ps
# Mid-point circles + terminal
plot -Sv0.2i+t0.1i+s+mc -X-5.25i -Y1.8i >> $ps
plot -Sv0.2i+t0.1i+s+mcl -X1.75i >> $ps
plot -Sv0.2i+t0.1i+s+mcr -X1.75i >> $ps
plot -Sv0.2i+t0.1i+s+mt -X1.75i >> $ps
# Mid-point terminals and squares
plot -Sv0.2i+t0.1i+s+mtl -X-5.25i -Y1.8i >> $ps
plot -Sv0.2i+t0.1i+s+mtr -X1.75i >> $ps
plot -Sv0.2i+t0.1i+s+ms -X1.75i >> $ps
plot -Sv0.2i+t0.1i+s+msl -X1.75i >> $ps
# Vectors at various ends with extra trim
plot -Sv0.2i+t0.1i+s+msr -X-5.25i -Y1.8i >> $ps
plot -Sv0.2i+t0.15i+s+e -X1.75i >> $ps
plot -Sv0.2i+t0.15i+s+b -X1.75i >> $ps
plot -Sv0.2i+t0.15i+s+b+e -X1.75i >> $ps
gmt psxy -R -J -O -T >> $ps
gv $ps &
