#!/bin/bash
#	$Id$
#
# Plot lines with variable number of NaNs

ps=plot_TS.ps

gmt psxy chkPts_tseries.dat -i0,12 -JX24c/16c -Bxa24f3 -Bya2f1 -BWS -R0/324/12/26 -W1 -K > $ps
gmt psxy chkPts_tseries.dat -i0,11 -JX -R -W1,red -O >> $ps

