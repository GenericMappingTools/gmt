#!/bin/sh
#	$Id: allsymbinch.sh,v 1.1 2011-02-27 22:59:21 guru Exp $
#
# Plot psxy symbols under INCH default unit

. ../functions.sh
header "Test psxy symbols for INCH environment"

ps=inch.ps

grep -v '^#' all_psxy_symbols.txt | awk '{print $1, $2}' | psxy -R0/4/0/6 -B1g1WSne:."MEASURE_UNIT=inch": -Jx1.25i -Sc1i -W0.25p,blue,. -P -K --MEASURE_UNIT=inch -Xc -Yc > $ps
psxy all_psxy_symbols.txt -R -J -S -Gred -W1p -O --MEASURE_UNIT=inch >> $ps

pscmp
