#!/bin/bash
#	$Id$
#
# Plot gmt psxy symbols under INCH default unit

ps=allsymbinch.ps

gmt psxy all_psxy_symbols.txt -R0/4/0/6 -B1g1 -BWSne+t"PROJ_LENGTH_UNIT=inch" -Jx1.25i -Sc1i -W0.25p,blue,. -P -K --PROJ_LENGTH_UNIT=inch -Xc -Yc -i0,1 > $ps
gmt psxy all_psxy_symbols.txt -R -J -S -Gred -W1p -O --PROJ_LENGTH_UNIT=inch >> $ps

