#!/bin/bash
#	$Id$
#
# Description: 

PS=pssac_F.ps

gmt pssac seis.sac -R9/20/-2/2 -JX15c/6c -B1 -W1p,black -Fr -K -P > $PS
gmt pssac seis.sac -R9/20/-0.06/0.06 -JX15c/6c -Bx1 -By0.02 -W1p,black -Fri -Y7c -K -O >> $PS
gmt pssac seis.sac -R9/20/-8e-3/8e-3 -JX15c/6c -Bx1 -By2e-3 -W1p,black -Friri -Y7c -K -O >> $PS
gmt pssac seis.sac -R9/20/-0.1/3 -JX15c/4c -B1 -W1p,black -Frq -Y7c -K -O >> $PS

gmt psxy -R$R -J$J -O -T >> $PS
