#!/bin/bash
#	$Id$
#
# Description:

PS=pssac_Q.ps

gmt pssac seis.sac -JX10c/5c -R9/20/-2/2 -Bx2 -By1 -BWSen -K -P \
    -Fr -Gp+gblack -Gn+gblue > $PS
gmt pssac seis.sac -JX5c/-10c -R-2/2/9/20 -Bx1 -By2 -BWSen -K -O \
    -Fr -Gp+gblack -Gn+gblue -Q -X12c >> $PS

gmt psxy -J -R -T -O >> $PS
