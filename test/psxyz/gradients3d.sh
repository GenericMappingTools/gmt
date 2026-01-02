#!/usr/bin/env bash
# GMT_KNOWN_FAILURE
#
# Test gradient-filled 3D polygons with -G+g (Gouraud shading)
# Tests RGB format (x y z r g b), color names (x y z colorname),
# and CPT format (x y z value)

ps=gradients3d.ps

# Create CPT for CPT-based tests
gmt makecpt -Chot -T0/100 > t.cpt

# Row 1: RGB format - triangle and square at z=0
gmt psxyz -R-1/4/-1/3/0/3 -JX6c -JZ4c -p135/30 -Bxafg -Byafg -Bzafg -BWSneZ+t"RGB Triangle" -G+g -P -K -X1.5 -Y19c << EOF > $ps
0 0 0 255 0 0
3 0 0 0 255 0
1.5 2.6 0 0 0 255
0 0 0 255 0 0
EOF

gmt psxyz -R-0.5/2.5/-0.5/2.5/0/3 -JX6c -JZ4c -p135/30 -Bxafg -Byafg -Bzafg -BWSneZ+t"RGB Square" -G+g -O -K -X9.5c << EOF >> $ps
0 0 1 255 0 0
2 0 1 255 255 0
2 2 1 0 255 0
0 2 1 0 0 255
0 0 1 255 0 0
EOF

# Row 2: Color names - triangle and square at different z levels
gmt psxyz -R-1/4/-1/3/0/3 -JX6c -JZ4c -p135/30 -Bxafg -Byafg -Bzafg -BWSneZ+t"Color Names Triangle" -G+g -O -K -X-9.5c -Y-8.5c << EOF >> $ps
0 0 0 red
3 0 0.5 green
1.5 2.6 1 blue
0 0 0 red
EOF

gmt psxyz -R-0.5/2.5/-0.5/2.5/0/3 -JX6c -JZ4c -p135/30 -Bxafg -Byafg -Bzafg -BWSneZ+t"Color Names Square" -G+g -O -K -X9.5c << EOF >> $ps
0 0 0.5 red
2 0 1 yellow
2 2 1.5 green
0 2 1 blue
0 0 0.5 red
EOF

# Row 3: CPT format - triangle and square
gmt psxyz -R-1/4/-1/3/0/3 -JX6c -JZ4c -p135/30 -Bxafg -Byafg -Bzafg -BWSneZ+t"CPT Triangle" -G+g -Ct.cpt -O -K -X-9.5c -Y-8.5c << EOF >> $ps
0 0 0 0
3 0 1 50
1.5 2.6 2 100
0 0 0 0
EOF

gmt psxyz -R-0.5/2.5/-0.5/2.5/0/3 -JX6c -JZ4c -p135/30 -Bxafg -Byafg -Bzafg -BWSneZ+t"CPT Square" -G+g -Ct.cpt -O -X9.5c << EOF >> $ps
0 0 0.5 0
2 0 1 33
2 2 1.5 67
0 2 1 100
0 0 0.5 0
EOF

\rm t.cpt
