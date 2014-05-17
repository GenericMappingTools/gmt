#!/bin/bash
# $Id$
# Test gmt pstext with text clip delay

ps=delay.ps
FONT=24p,Helvetica

gmt pstext -R0/10/0/10 -JX15c/10c -F+f${FONT}+jCM -GC -B0 -B+glightbrown -C0 -K -P << EOF > $ps
5 5 This text is invisible
EOF

# Overlay big polygon
gmt psxy -R -J -L -Glightblue -W0.5p -O -K << EOF >> $ps
1 1
9 1
9 9
1 9
EOF

gmt psclip -C -O -K >> $ps

gmt pstext -R -J -F+f${FONT}+jCM -Gc -B0 -B+glightbrown -C0 -O -K -Y12c << EOF >> $ps
5 5 This text is plotted then clipped
EOF

# Overlay big polygon
gmt psxy -R -J -L -Glightblue -W0.5p -O -K << EOF >> $ps
1 1
9 1
9 9
1 9
EOF

gmt psclip -C -O >> $ps
