#!/bin/bash
#	$Id: plot_TS.sh,v 1.2 2011-03-15 02:06:46 guru Exp $
#
# Plot lines with variable number of NaNs

. ../functions.sh
header "Test psxy with gaps"

ps=TS_sst.ps

psxy chkPts_tseries.dat -i0,12 -JX26c/16c -Ba12f3/a2f1WS -R0/324/12/26 -W1 -K > $ps
psxy chkPts_tseries.dat -i0,11 -JX -R -W1,red -O >> $ps

pscmp
