#!/usr/bin/env bash
#

ps=seis_02.ps

gmt set PROJ_LENGTH_UNIT inch MAP_TICK_LENGTH_PRIMARY 0.075i MAP_FRAME_WIDTH 0.1i MAP_ORIGIN_X 2.5c MAP_ORIGIN_Y 1.3i

# This is a strike-slip CMT mechanism in red
# Best double couple overlays moment tensor
gmt psmeca -X2i -Y5i -R238.5/242/32.5/35.5 -Jm1.3i -B2 -BWeSn+tpsmeca -Sm0.4i/12pu -T0 -P -C0.5pP5p -Gred -K -W0.75p << EOF > $ps
# lon   lat     dep mrr  mtt   mff  mrt  mrf   mtf  exp plon  plat text
239.384 34.556   33 -.27 -2.13 2.40 -.07 -1.32 -.79 24  240.0 35   tensor
EOF

# Focal mechanisms are labeled with following font
gmt set FONT_ANNOT_PRIMARY Times-Roman
# Here are several thrusting mechanisms in Aki and Richards convention
# New second argument -Sa is font size in points
# Old gmt psvelomeca format is used

gmt psmeca -Fo -R -J -Sa0.4i/16p -C0.25p,blueP5p -O -K -N << EOF >> $ps
# lon    lat        strike dip   rake   Mw      plon plat text
241.459  34.2088    112.3  42.2  89.8   6.6     238. 35.0 first
241.459  34.2088    120.0  60.0  86.0   5.2     238. 34.0 second
241.459  34.2088    290.0  55.0  90.0   5.9     238. 33.0 third
EOF

# Plot P and T axis and only the best double couple
# derived from moment tensor
gmt psmeca -R -J -Sd1c -O -K -N -Fa0.2c/id << EOF >> $ps
# lon   lat   dep  mrr  mtt   mff  mrt  mrf   mtf  exp   plon plat
  241   33    3   -.27 -2.13 2.40 -.07 -1.32 -.79  26      0    0
EOF

# hit the beach
gmt pscoast -O -R -J -W1p -Di >> $ps

