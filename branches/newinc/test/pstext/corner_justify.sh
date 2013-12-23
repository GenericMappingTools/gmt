#!/bin/bash
#	$Id$
#
# Test gmt pstext with coordinates extracted from -R

function plot {
	gmt pstext -R -J --FONT=24p,Helvetica-Bold -O -K $*
}

ps=corner_justify.ps

gmt psbasemap -R0/16/0/12 -Jx1c -B5g1 -BWSne -P -K > $ps
echo TopLeft   | plot -F+cTL >> $ps
echo MidLeft   | plot -F+cML >> $ps
echo BotLeft   | plot -F+cBL >> $ps
echo TopCenter | plot -F+cTC >> $ps
echo MidCenter | plot -F+cMC >> $ps
echo MidCenter | plot -D1c -F+cMC >> $ps
echo BotCenter | plot -F+cBC >> $ps
echo TopRight  | plot -F+cTR >> $ps
echo MidRight  | plot -F+cMR >> $ps
echo BotRight  | plot -F+cBR >> $ps
gmt psxy -R -J -O -T >> $ps
