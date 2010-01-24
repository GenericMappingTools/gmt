#!/bin/sh
#	$Id: meca_1.sh,v 1.1 2010-01-24 19:53:51 remko Exp $
#

. ../functions.sh
header "Test psmeca for plotting focal mechanisms (1)"

ps=meca_1.ps

gmtset MEASURE_UNIT inch TICK_LENGTH 0.075i FRAME_WIDTH 0.1i X_ORIGIN 2.5c Y_ORIGIN 1.3i

# This is a strike-slip CMT mechanism in red
# Best double couple overlays moment tensor
psmeca -X2i -Y5i -R238.5/242/32.5/35.5 -Jm1.3i -B2/2/:.psmeca:WeSn -Sm0.4i/12u -H1 -T0 -P -C0.50p/0/0/0 -G255/0/0 -K -W0.75p/0 << EOF > $ps
lon     lat     dep mrr  mtt   mff  mrt  mrf   mtf  exp plon plat
239.384 34.556   33 -.27 -2.13 2.40 -.07 -1.32 -.79 24  240.0 35  tensor
EOF

# Focal mechanisms are labeled with following font
gmtset ANOT_FONT Times-Roman
# Here are several thrusting mechanisms in Aki and Richards convention
# New second argument -Sa is font size in points
# Old psvelomeca format is used

psmeca -o -R -J -Sa0.4i/16 -H1 -C0.25p/0/0/255 -O -K -N << EOF >> $ps
* lon    lat         strike  dip  rake   Mw     plon plat
241.459  34.2088    112.3  42.2  89.8   6.6     238. 35.0 first
241.459  34.2088    120.0  60.0  86.0   5.2     238. 34.0 second
241.459  34.2088    290.0  55.0  90.0   5.9     238. 33.0 third
EOF
 
# Plot P and T axis and only the best double couple
# derived from moment tensor
psmeca -R -J -Sd1c -H1  -O -K -N -a0.2c/id << EOF >> $ps
* lon   lat   dep  mrr  mtt   mff  mrt  mrf   mtf  exp   plon plat
  241   33    3   -.27 -2.13 2.40 -.07 -1.32 -.79  26      0    0
EOF

# hit the beach
pscoast -O -R -J -W1 -Di >> $ps

rm -f .gmtcommands4

pscmp
