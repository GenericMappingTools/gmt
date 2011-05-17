#!/bin/bash
#
#	$Id: origin.sh,v 1.1 2011-05-17 01:37:55 remko Exp $

. ../functions.sh
header "Test positioning with -X and -Y"

ps=origin.ps
n=0

plot () {
	let n=$n+1
	pstext -R0/1/0/1 -JX1i -B0 -F+jMC $1 $2 $3 <<%
0.5 0.3 $1
0.5 0.7 ($n)
%
}

plot "-Xa1i -Ya2i" -P -K > $ps
plot "-X2i -Y2i" -O -K >> $ps
plot "-Xc -Yc" -O -K >> $ps
plot -X-2i -O -K >> $ps
plot "-Ya-2i" -O -K >> $ps
plot "-Y-3i" -O >> $ps

pscmp
