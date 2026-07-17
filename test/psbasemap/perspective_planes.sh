#!/usr/bin/env bash
#
# Test 3D perspective basemap with -px, -py, -pz view planes side by side.
# Each cube uses -BwsENZ1+b to draw all 12 cube edges. Y range 0-20, X range 0-10, Z range 0-30.
# -px: looking along X (plan = YZ); -py: looking along Y (plan = XZ); -pz: looking along Z (plan = XY).

ps=perspective_planes.ps

common="-R0/10/0/20/0/30 -JX2.5c/5c -JZ7.5c -Bxa+lx -Bya+ly -Bza+lz -BwsENZ1+b --PS_MEDIA=A4"

gmt psbasemap $common -px135/40/5  -P -K -Xf1c   -Yf2c > $ps
gmt psbasemap $common -py135/40/10    -O -K -Xf8c        >> $ps
gmt psbasemap $common -pz135/40/20    -O    -Xf14c       >> $ps
