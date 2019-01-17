#!/usr/bin/env bash
#
# Test new custom symbol macros using a windbarb.def file

ps=windbarb.ps

cat << EOF > tmp
150	-40	25	81.0
150	-30	50	0.0
EOF

ln -fs "${GMT_SRCDIR:-.}"/windbarb.def .

# Mercator
gmt pscoast -JM5i -R110/165/-45/-10 -Gwheat -Sazure1 -Wthin,black -Bafg -BWSen -K -P -Xc > $ps
gmt psxy -J -R -W1p,black -Gblack -L -Skwindbarb/1.5i -O -K tmp >> $ps
# Stereographic
gmt pscoast -JS120/-90/5i -R -Gwheat -Sazure1 -Wthin,black -Bafg -BWSen -O -K -Y5i >> $ps
gmt psxy -J -R -W2p,red -Gblack -L -Skwindbarb/1.5i -O tmp >> $ps

