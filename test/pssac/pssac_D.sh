#!/bin/bash
#	$Id$
#
# Description:

R=9/20/-2/2
J=X15c/5c
B=1
PS=pssac_D.ps

gmt pssac seis.sac -R$R -J$J -B$B -K -P > $PS
gmt pssac seis.sac -R$R -J$J -B$B -Wred -K -O -D0.2c/0.2c >> $PS
gmt psxy -R$R -J$J -T -O >> $PS
