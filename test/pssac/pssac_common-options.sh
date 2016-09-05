#!/bin/bash
#	$Id$
#
# Description:

J=X15c/5c
R=9/20/-2/2
Bx=x2
By=y1
ps=pssac_common-options.ps
SACFILEs="${src:-.}"/seis.sac

gmt pssac -J$J -R$R -B$Bx -B$By -B+t"Option -P for portrait mode" -K -P $SACFILEs > $ps
gmt pssac -J$J -R$R -B$Bx -B$By -B+t"Option -X and -Y" -Y10c -Xa1c -K -O $SACFILEs >> $ps
gmt pssac -J$J -R$R -B$Bx -B$By -B+t"Option -U and -t" -Y10c -K -O -t50 $SACFILEs >> $ps

gmt psxy -J$J -R$R -T -O >> $ps
