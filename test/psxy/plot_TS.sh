#!/bin/bash
#	$Id: plot_TS.sh,v 1.3 2011-03-22 16:37:13 remko Exp $
#
# Plot lines with variable number of NaNs

. ../functions.sh
header "Test psxy with gaps"

ps=plot_TS.ps

psxy chkPts_tseries.dat -i0,12 -JX24c/16c -Ba24f3/a2f1WS -R0/324/12/26 -W1 -K > $ps
psxy chkPts_tseries.dat -i0,11 -JX -R -W1,red -O >> $ps

pscmp
