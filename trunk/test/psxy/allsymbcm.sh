#!/bin/sh
#	$Id: allsymbcm.sh,v 1.2 2011-02-27 23:01:07 guru Exp $
#
# Plot psxy symbols under CM default unit

. ../functions.sh
header "Test psxy symbols for CM environment"

ps=cm.ps

grep -v '^#' all_psxy_symbols.txt | awk '{print $1, $2}' | psxy -R0/4/0/6 -B1g1WSne:."MEASURE_UNIT=cm": -Jx1.25i -Sc1i -W0.25p,blue,. -P -K --MEASURE_UNIT=cm -Xc -Yc > $ps
psxy all_psxy_symbols.txt -R -J -S -Gred -W1p -O --MEASURE_UNIT=cm >> $ps

pscmp
