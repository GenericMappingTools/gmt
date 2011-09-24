#!/bin/csh -xe
#	$Id$
\rm -f .gmtdefaults4
gmtset MEASURE_UNIT inch TICK_LENGTH 0.075i \
    FRAME_WIDTH 0.1i X_ORIGIN 2.5c Y_ORIGIN 1.3i DEGREE_FORMAT 3
set frame =  -R238.5/242/32.5/35.5
set proj = -Jm1.3i 

# This is a strike-slip CMT mechanism in red
# Best double couple overlays moment tensor
psmeca -X2 -Y5 $frame $proj -B2/2/:.psmeca:WeSn \
    -Sm0.4i/12u -h1 -T0 -P -C0.50p/0/0/0 -G255/0/0 -K -W0.75p/0 \
     << EOF >! meca_1.ps
lon     lat     dep mrr  mtt   mff  mrt  mrf   mtf  exp plon plat
239.384 34.556   33 -.27 -2.13 2.40 -.07 -1.32 -.79 24  240.0 35  tensor
EOF

# Focal mechanisms are labeled with following font
gmtset ANOT_FONT  Times-Roman
# Here are several thrusting mechanisms in Aki and Richards convention
# New second argument -Sa is font size in points
# Old psvelomeca format is used

alias psvelomeca "psmeca -o"
psvelomeca $frame $proj -Sa0.4i/16 -h1 -C0.25p/0/0/255 -O -K \
    -N -P << EOF >> meca_1.ps
* lon    lat         strike  dip  rake   Mw     plon plat
241.459  34.2088    112.3  42.2  89.8   6.6     238. 35.0 first
241.459  34.2088    120.0  60.0  86.0   5.2     238. 34.0 second
241.459  34.2088    290.0  55.0  90.0   5.9     238. 33.0 third
EOF
 
# Plot P and T axis and only the best double couple
# derived from moment tensor
psmeca $frame $proj -Sd1c -h1  -O -K \
-N -P  -a0.2c/id << EOF >> meca_1.ps
* lon   lat   dep  mrr  mtt   mff  mrt  mrf   mtf  exp   plon plat
  241   33    3   -.27 -2.13 2.40 -.07 -1.32 -.79  26      0    0
EOF

# hit the beach
pscoast -P -O $frame $proj -W1 -Di >> meca_1.ps
\rm -f .gmtdefaults4
