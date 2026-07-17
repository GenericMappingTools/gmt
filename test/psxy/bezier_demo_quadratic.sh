#!/usr/bin/env bash
#
# Test Q (quadratic Bezier) command in custom symbol macro language.
# Uses share/custom/leaf.def (closed leaf shape from two Q curves)
# and qbezier_test.def (open parabolic arc).

ps=bezier_demo_quadratic.ps

gmt psbasemap -R-5/5/-4/4 -JX14c/10c -Ba2f1 -BWeSn+t"Quadratic Bezier (Q) custom symbol demo" -P -K > $ps

# Grid of leaves at different sizes and colors
for lon in -4 -2 0 2 4; do
	echo "$lon  2" | gmt psxy -R -J -Skleaf/1.5c -Glightgreen -W0.5p,darkgreen -O -K >> $ps
	echo "$lon  0" | gmt psxy -R -J -Skleaf/2.0c -Ggreen      -W0.7p,darkgreen -O -K >> $ps
	echo "$lon -2" | gmt psxy -R -J -Skleaf/2.5c -Gdarkgreen  -W1p,black       -O -K >> $ps
done

cat > qbezier_test.def << EOF
# Test quadratic Bezier Q command
# Draws a simple parabolic arc from (-0.5,0) to (0.5,0) curving up to (0,0.5)
# M = moveto start, Q = quadratic bezier, S = stroke
-0.5  0.0  M
 0.0  0.5  0.5  0.0  Q
S
EOF

# Parabolic arc symbol (open path, just stroked)
for lon in -3 -1 1 3; do
	echo "$lon  3.2" | gmt psxy -R -J -Skqbezier_test/1.5c -W1.5p,red -O -K >> $ps
done

# Labels
gmt pstext -R -J -F+f11p,Helvetica-Bold,red+jCM -O -K >> $ps << EOF
0 3.7 Parabolic arcs
EOF
gmt pstext -R -J -F+f9p+jCM -O >> $ps << EOF
0 2.7 1.5c leaves
0 0.7 2.0c leaves
0 -1.3 2.5c leaves
EOF
rm qbezier_test.def
