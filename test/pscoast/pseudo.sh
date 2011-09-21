#!/bin/bash

# GMT 5 test script for pseudo perspective on some projections. By Ken McLean.
# Create normal plot, 180 deg rotation, and oblique pseudo perspective of each.

ps=pseudo.ps

. ../functions.sh
header "Test placement of pseudo perspective projections"

coast () {
pscoast -B+glightblue -Dc -Gblack -O -K -Ya0c -p180/90 $*
pscoast -B+glightblue -Dc -Gblack -O -K -Ya3.5c -p0/90 $*
pscoast -B+glightblue -Dc -Gblack -O -K -Ya7c -p135/45 $*
}

basemap () {
psbasemap -B+glightblue -O -K -Ya0c -p180/90 $*
psbasemap -B+glightblue -O -K -Ya3.5c -p0/90 $*
psbasemap -B+glightblue -O -K -Ya7c -p135/45 $*
}

# TOP ROW
psxy -Rg -JX1c -T -K -Yf10.5c > $ps
# Transverse Mercator
coast -R0/360/-80/80 -JT330/-45/3c -Bg15 -A10000 -Xf0c >> $ps

# Cartesian linear
basemap -R0/5/0/5 -JX3cd/3c -Bg1 -Xf4c >> $ps

# Stereographic Equal-Angle
coast -R-30/30/60/72 -JS0/90/3c -Bg5 -A10000 -Xf8c >> $ps

# Oblique Mercator 
coast -R270/20/305/25r -JOc280/25.5/22/69/3c -Bg5 -A10 -Xf12c >> $ps

# Cassini cylindrical
coast -Rg -JQ3c -Bg30 -A10000 -Xf16c >> $ps

# Eckert
coast -Rg -JK3c -Bg30 -A10000 -Xf20c >> $ps

# Mollweide
coast -Rg -JW3c -Bg30 -A10000 -Xf24c >> $ps

# BOTTOM ROW
psxy -Rg -JX1c -T -O -K -Yf0c >> $ps
# Gnomonic
coast -Rg -JF-120/35/60/3c -Bg15 -A10000 -Xf0c >> $ps

# Polar
coast -R0/360/0/90 -JP3c -Bg15 -A10000 -Xf4c >> $ps

# American polyconic
coast -R-180/-20/0/90 -JPoly/3c -Bg15 -A10000 -Xf8c >> $ps

# Lambert
coast -Rg -JA280/30/3c -Bg15 -A10000 -Xf12c >> $ps

# Orthographic
coast -Rg -JG-75/41/3c -Bg15 -A10000 -Xf16c >> $ps

# Azimuthal equidistant
coast -Rg -JE-100/40/3c -Bg15 -A10000 -Xf20c >> $ps

# Van der Grinten
coast -Rg -JV3c -Bg15 -A10000 -Xf24c >> $ps

psxy -R -J -T -O >> $ps

pscmp
