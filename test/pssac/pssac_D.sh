#!/bin/bash
#
# Description:

R=9/20/-2/2
J=X15c/5c
B=1
ps=pssac_D.ps

gmt pssac seis.sac -R$R -J$J -B$B -K -P > $ps
gmt pssac seis.sac -R -J -Wred -O -D0.2c/0.2c >> $ps
