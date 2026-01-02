#!/usr/bin/env bash
# GMT_KNOWN_FAILURE
#
# Test gradient-filled polygons with -G+g (Gouraud shading)
# Tests RGB format (x y r g b), color names (x y colorname),
# CPT format (x y z), and arbitrary polygon vertex counts

ps=gradients.ps

# Create CPT for CPT-based tests
gmt makecpt -Chot -T0/100 > t.cpt

# Row 1: RGB format - triangle and square
gmt psxy -R-1/4/-1/3 -JX4c -Bafg -BWSne+t"RGB Triangle" -G+g -P -K -Xc -Y18c << EOF > $ps
0 0 255 0 0
3 0 0 255 0
1.5 2.6 0 0 255
0 0 255 0 0
EOF

gmt psxy -R-0.5/2.5/-0.5/2.5 -JX4c -Bafg -BWSne+t"RGB Square" -G+g -O -K -X5c << EOF >> $ps
0 0 255 0 0
2 0 255 255 0
2 2 0 255 0
0 2 0 0 255
0 0 255 0 0
EOF

# Row 2: Color names - triangle and square
gmt psxy -R-1/4/-1/3 -JX4c -Bafg -BWSne+t"Color Names Triangle" -G+g -O -K -X-5c -Y-5.5c << EOF >> $ps
0 0 red
3 0 green
1.5 2.6 blue
0 0 red
EOF

gmt psxy -R-0.5/2.5/-0.5/2.5 -JX4c -Bafg -BWSne+t"Color Names Square" -G+g -O -K -X5c << EOF >> $ps
0 0 red
2 0 yellow
2 2 green
0 2 blue
0 0 red
EOF

# Row 3: CPT format - triangle and square
gmt psxy -R-1/4/-1/3 -JX4c -Bafg -BWSne+t"CPT Triangle" -G+g -Ct.cpt -O -K -X-5c -Y-5.5c << EOF >> $ps
0 0 0
3 0 50
1.5 2.6 100
0 0 0
EOF

gmt psxy -R-0.5/2.5/-0.5/2.5 -JX4c -Bafg -BWSne+t"CPT Square" -G+g -Ct.cpt -O -K -X5c << EOF >> $ps
0 0 0
2 0 33
2 2 67
0 2 100
0 0 0
EOF

# Row 4: Pentagon (arbitrary polygon) with hex colors and slash-separated RGB
gmt psxy -R-0.5/2.5/-0.5/2 -JX4c -Bafg -BWSne+t"Pentagon Hex" -G+g -O -K -X-5c -Y-5.5c << EOF >> $ps
1 0 #FF0000
1.951 0.588 #FFA500
1.588 1.538 #FFFF00
0.412 1.538 #00FF00
0.049 0.588 #0000FF
1 0 #FF0000
EOF

gmt psxy -R-0.5/2.5/-0.5/2 -JX4c -Bafg -BWSne+t"Pentagon Slash RGB" -G+g -O -X5c << EOF >> $ps
1 0 255/0/0
1.951 0.588 255/165/0
1.588 1.538 255/255/0
0.412 1.538 0/255/0
0.049 0.588 0/0/255
1 0 255/0/0
EOF

\rm t.cpt