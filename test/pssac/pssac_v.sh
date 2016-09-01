#!/bin/bash
#	$Id$
#
# Description: 

PS=pssac_v.ps

gmt pssac -JX10c/5c -R9/20/-2/2 -Bx2 -By1 -BWSen -K -P \
    -Fr -Gp+gblack -Gn+gblue seis.sac -V > $PS
gmt pssac -JX5c/-10c -R-2/2/9/20 -Bx1 -By2 -BWSen -K -O \
    -Fr -Gp+gblack -Gn+gblue seis.sac -v -X12c -V >> $PS

gmt psxy -J$J -R$R -T -O >> $PS
