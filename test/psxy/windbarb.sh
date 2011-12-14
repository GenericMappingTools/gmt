#!/bin/bash
#	$Id$
#
# Test new custom symbol macros using a windbarb.def file

. ../functions.sh
header "Test psxy with multi-parameter custom symbol"

ps=barb.ps

pscoast -JM6i -R110/160/-45/-10 -Gwheat -Sazure1 -Wthin,black -B:."Wind barb symbol": -Bf10a30/f10a30WSen -K -P > $ps
psxy -J -R -Wthinnest,black -Gblack -L -Skwindbarb/1.5i -O << EOF >> $ps
150	-40	25	9.0
150	-30	50	90.0
EOF

pscmp
