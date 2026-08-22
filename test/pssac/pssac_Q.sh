#!/usr/bin/env bash
#
# Description:

ps=pssac_Q.ps

gmt pssac seis.sac -JX10c/5c -R9/20/-2/2 -Bx2 -By1 -BWSen -K -P \
    -Fr -Gp+gblack -Gn+gblue -W > $ps
gmt pssac seis.sac -JX5c/-10c -R-2/2/9/20 -Bx1 -By2 -BWSen -O \
    -Fr -Gp+gblack -Gn+gblue -W -Q -X12c >> $ps
