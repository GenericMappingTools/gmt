#!/bin/bash
#	$Id: allsymbinch.sh,v 1.4 2011-04-26 19:29:44 remko Exp $
#
# Plot psxy symbols under INCH default unit

. ../functions.sh
header "Test psxy symbols for INCH environment"

ps=allsymbinch.ps

psxy all_psxy_symbols.txt -R0/4/0/6 -B1g1WSne:."PROJ_LENGTH_UNIT=inch": -Jx1.25i -Sc1i -W0.25p,blue,. -P -K --PROJ_LENGTH_UNIT=inch -Xc -Yc -i0,1 > $ps
psxy all_psxy_symbols.txt -R -J -S -Gred -W1p -O --PROJ_LENGTH_UNIT=inch >> $ps

pscmp
