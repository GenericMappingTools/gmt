#!/usr/bin/env bash
#
# Description:

J=X15c/5c
R=9/20/-2/2
Bx=x2
By=y1
ps=pssac_common-options.ps

gmt set PS_MEDIA 21cx35c
gmt pssac seis.sac -J$J -R$R -B$Bx -B$By -B+t"Option -P for portrait mode" -K -P > $ps
gmt pssac seis.sac -J$J -R$R -B$Bx -B$By -B+t"Option -X and -Y" -Y8c -Xa1c -K -O >> $ps
gmt pssac seis.sac -J$J -R$R -B$Bx -B$By -B+t"Option -t" -Y8c -K -O -t100 >> $ps
gmt pssac seis.sac -J$J -R$R -B$Bx -B$By -B+t"Option -p" -Y8c -O -p170/30 >> $ps
