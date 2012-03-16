#!/bin/bash
#	$Id$
#
# Plot lines with variable number of NaNs

header "Test psxy with gaps"

psxy chkPts_tseries.dat -i0,12 -JX24c/16c -Ba24f3/a2f1WS -R0/324/12/26 -W1 -K > $ps
psxy chkPts_tseries.dat -i0,11 -JX -R -W1,red -O >> $ps

pscmp
