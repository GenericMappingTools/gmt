#!/usr/bin/env bash
#
# Test C (cubic Bezier) command in custom symbol macro language.
# Uses share/custom/heart.def (closed heart shape via two C curves)
# alongside share/custom/leaf.def (Q-based) for visual comparison.

ps=bezier_demo_cubic.ps

gmt psbasemap -R-5/5/-4/4 -JX14c/10c -Ba2f1 -BWeSn+t"Cubic Bezier (C) custom symbol demo" -P -K > $ps

# Hearts in a row, different sizes and colors
for lon in -4 -2 0 2 4; do
	echo "$lon  2" | gmt psxy -R -J -Skheart/1.5c -Gred     -W0.5p,darkred -O -K >> $ps
	echo "$lon  0" | gmt psxy -R -J -Skheart/2.0c -Gtomato  -W0.7p,red     -O -K >> $ps
	echo "$lon -2" | gmt psxy -R -J -Skheart/2.5c -Gdarkred -W1p,black     -O -K >> $ps
done

# Compare: leaves (Q) vs hearts (C) side by side
for lon in -3.5 -1.5 0.5 2.5; do
	echo "$lon  3.3" | gmt psxy -R -J -Skleaf/1.2c  -Ggreen -W0.5p,darkgreen -O -K >> $ps
done
for lon in -2.5 -0.5 1.5 3.5; do
	echo "$lon  3.3" | gmt psxy -R -J -Skheart/1.2c -Gred   -W0.5p,darkred   -O -K >> $ps
done

echo "0 3.8 Q (leaf) vs C (heart)" | gmt pstext -R -J -F+f10p,Helvetica-Bold+jCM -O >> $ps
