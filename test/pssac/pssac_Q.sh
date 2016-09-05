#!/bin/bash
#	$Id$
#
# Description: 

PS=pssac_Q.ps

gmt pssac -JX10c/5c -R9/20/-2/2 -Bx2 -By1 -BWSen -K -P \
    -Fr -Gp+gblack -Gn+gblue "${src:-.}"/seis.sac -V > $PS
gmt pssac -JX5c/-10c -R-2/2/9/20 -Bx1 -By2 -BWSen -K -O \
    -Fr -Gp+gblack -Gn+gblue "${src:-.}"/seis.sac -Q -X12c -V >> $PS

gmt psxy -J -R -T -O >> $PS
