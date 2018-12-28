#!/bin/bash
#	$Id$
#
# Description:

ps=pssac_C.ps
gmt pssac ntkl.z onkl.z -JX15c/4c -R200/1600/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -P > $ps
gmt pssac ntkl.z onkl.z -JX15c/4c -R200/1600/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -C500/1100 -Y6c >> $ps
gmt pssac ntkl.z onkl.z -JX15c/4c -R-300/1100/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -T+t1 -Y6c >> $ps
gmt pssac ntkl.z onkl.z -JX15c/4c -R-300/1100/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -T+t1 -C-100/500 -Y6c >> $ps
gmt psxy -J -R -O -T >> $ps
