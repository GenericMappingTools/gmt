#!/bin/bash
#	$Id$
#
# Description: 

R=9/20/-2/2
J=X15c/5c
B=1
PS=pssac_W.ps

gmt pssac -R$R -J$J -B$B -W2p,red,- seis.sac -K -P > $PS
gmt pssac -R$R -J$J -B$B -W1p seis.sac -K -O -Y7c >> $PS
gmt pssac -R$R -J$J -B$B -Wblue seis.sac -K -O -Y7c >> $PS
gmt pssac -R$R -J$J -B$B -W. seis.sac -K -O -Y7c >> $PS

gmt psxy -R$R -J$J -T -K -O >> $PS
