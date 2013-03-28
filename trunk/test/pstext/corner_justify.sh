#!/bin/bash
#	$Id$
#
# Test pstext with coordinates extracted from -R

ps=corner_justify.ps


psbasemap -R0/16/0/12 -Jx1c -B5g1WSne -P -K > $ps
echo TopLeft | pstext -R -J --FONT=24p,Helvetica-Bold -F+cTL -O -K >> $ps
echo MidLeft | pstext -R -J --FONT=24p,Helvetica-Bold -F+cML -O -K >> $ps
echo BotLeft | pstext -R -J --FONT=24p,Helvetica-Bold -F+cBL -O -K >> $ps
echo TopCenter | pstext -R -J --FONT=24p,Helvetica-Bold -F+cTC -O -K >> $ps
echo MidCenter | pstext -R -J --FONT=24p,Helvetica-Bold -F+cMC -O -K >> $ps
echo MidCenter | pstext -R -J --FONT=24p,Helvetica-Bold -D1 -F+cMC -O -K >> $ps
echo BotCenter | pstext -R -J --FONT=24p,Helvetica-Bold -F+cBC -O -K >> $ps
echo TopRight | pstext -R -J --FONT=24p,Helvetica-Bold -F+cTR -O -K >> $ps
echo MidRight | pstext -R -J --FONT=24p,Helvetica-Bold -F+cMR -O -K >> $ps
echo BotRight | pstext -R -J --FONT=24p,Helvetica-Bold -F+cBR -O >> $ps
