#!/bin/bash
#	$Id$
#
# Description:

R=9/20/-2/2
J=X15c/5c
B=1
PS=pssac_W.ps

gmt pssac seis.sac -R$R -J$J -B$B -W2p,red,- -K -P > $PS
gmt pssac seis.sac -R$R -J$J -B$B -W1p -K -O -Y7c >> $PS
gmt pssac seis.sac -R$R -J$J -B$B -Wblue -K -O -Y7c >> $PS
gmt pssac seis.sac -R$R -J$J -B$B -W. -K -O -Y7c >> $PS
gmt psxy -R$R -J$J -T -K -O >> $PS
