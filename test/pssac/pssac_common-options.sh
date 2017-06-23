#!/bin/bash
#	$Id$
#
# Description:

J=X15c/5c
R=9/20/-2/2
Bx=x2
By=y1
PS=pssac_common-options.ps

gmt pssac seis.sac -J$J -R$R -B$Bx -B$By -B+t"Option -P for portrait mode" -K -P > $PS
gmt pssac seis.sac -J$J -R$R -B$Bx -B$By -B+t"Option -X and -Y" -Y8c -Xa1c -K -O >> $PS
gmt pssac seis.sac -J$J -R$R -B$Bx -B$By -B+t"Option -t" -Y8c -K -O -t100 >> $PS
gmt psxy -J -R -T -O >> $PS
