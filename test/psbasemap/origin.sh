#!/usr/bin/env bash
#

ps=origin.ps

n=0

plot () {
	let n=$n+1
	gmt pstext --GMT_VERBOSE=0 -R0/1/0/1 -JX1i -B0 -F+jMC $1 $2 $4 $3 <<%
0.5 0.8 ($n)
0.5 0.5 $1
0.5 0.2 $2
%
}

plot -Xa1i -Ya2i -P -K > $ps
plot -X2i -Y2i -O -K >> $ps
plot -Xc -Yc -O -K >> $ps
plot -X-2i "" -O -K >> $ps
plot -Ya-2i "" -O -K >> $ps
plot -X1i "" -O -K >> $ps
plot -Xc1i -Y-1i -O -K >> $ps
plot -X -Y -O -K >> $ps
plot -Xf6i -Y -O -K >> $ps
plot -X -Y -O -K >> $ps
plot -Xf4.75i -Yf5i -O -K >> $ps
gmt psxy -Xf0 -Yf0 -R0/1/0/1 -JX8.5i/11i -Wthinnest,red -O >> $ps <<%
0 0
1 1
>
0 1
1 0
%

