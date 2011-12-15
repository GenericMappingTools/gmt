#!/bin/bash
#	$Id$
#
# Test new custom symbol macros using a windbarb.def file

. ../functions.sh
header "Test psxy with multi-parameter windbarb symbol"

ps=barb.ps
cat << EOF > tmp
150	-40	25	9.0
150	-30	50	90.0
EOF

# Mercator
pscoast -JM5i -R110/165/-45/-10 -Gwheat -Sazure1 -Wthin,black -BafgWSen -K -P -Xc > $ps
psxy -J -R -W1p,black -Gblack -L -Skwindbarb/1.5i -O -K tmp >> $ps
# Stereographic
pscoast -JS120/-90/5i -R -Gwheat -Sazure1 -Wthin,black -BafgWSen -O -K -Y5i >> $ps
psxy -J -R -W2p,red -Gblack -L -Skwindbarb/1.5i -O tmp >> $ps

rm -f tmp
pscmp
