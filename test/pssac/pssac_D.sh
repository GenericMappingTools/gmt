#!/bin/bash
#	$Id$
#
# Description: 

R=9/20/-2/2
J=X15c/5c
B=1
ps=pssac_D.ps

gmt pssac -R$R -J$J -B$B seis.sac -K -P > $ps
gmt pssac -R$R -J$J -B$B seis.sac -Wred -K -O -D0.2c/0.2c >> $ps
gmt psxy -R$R -J$J -T -O >> $ps
