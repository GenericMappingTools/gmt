#!/bin/sh
#	$Id: allsymbcm.sh,v 1.3 2011-03-15 02:06:46 guru Exp $
#
# Plot psxy symbols under CM default unit

. ../functions.sh
header "Test psxy symbols for CM environment"

ps=cm.ps

psxy all_psxy_symbols.txt -R0/4/0/6 -B1g1WSne:."PROJ_LENGTH_UNIT=cm": -Jx1.25i -Sc1i -W0.25p,blue,. -P -K --PROJ_LENGTH_UNIT=cm -Xc -Yc -i0,1 > $ps
psxy all_psxy_symbols.txt -R -J -S -Gred -W1p -O --PROJ_LENGTH_UNIT=cm >> $ps

pscmp
