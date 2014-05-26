#!/bin/bash
# $Id$
# Test gmt pstext with text path clipping

ps=textclip.ps
FONT=16p,Helvetica

gmt pstext -R0/10/0/10 -JX15c/10c -F+f${FONT}+jBL -Gc -B+ggray70 -C0 -K -P << EOF > $ps
1 3 This text should NOT be covered up by the tan box
1 8 Neither should this jolly text message
EOF

# Overlay big polygon
gmt psxy -R -J -L -Gtan -W0.5p -O -K << EOF >> $ps
3 1
9 1
9 9
3 9
EOF

gmt psclip -C -O >> $ps

