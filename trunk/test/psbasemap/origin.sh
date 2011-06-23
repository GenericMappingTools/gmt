#!/bin/bash
#
#	$Id: origin.sh,v 1.5 2011-06-23 17:52:06 remko Exp $

. ../functions.sh
header "Test positioning with -X and -Y"

ps=origin.ps
n=0

plot () {
	let n=$n+1
	pstext --GMT_VERBOSE=0 -R0/1/0/1 -JX1i -B0 -F+jMC $1 $2 $4 $3 <<%
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
psxy -Xf0 -Yf0 -R0/1/0/1 -JX8.5i/11i -Wthinnest,red -O >> $ps <<%
0 0
1 1
>
0 1
1 0
%

pscmp
