#!/usr/bin/env bash
# Testing gmt spatial segment extensions via -W
# DVC_TEST
cat << EOF > t.txt
-0.5	1.5
0.25	2.0
0.75	2.0
1.25	3.0
EOF

gmt begin extend
	# Geographic
	gmt subplot begin 2x2 -Fs8c -A+gwhite+p0.25p+o5p -R-2/2/0/4 -M10p
		gmt subplot set 0 -A"Geographic: 60n/100k"
		gmt spatial t.txt -W60n/100k > new.txt
		gmt plot -JM? -W4p,gray new.txt -Bafg
		gmt plot t.txt -W1p
		gmt plot new.txt -Sc8p -Ggreen
		gmt plot t.txt -Sc8p -Gred
		gmt subplot set 1 -A"Geographic: 1d at start"
		gmt spatial t.txt -W1d+f > new.txt
		gmt plot -JM? -W4p,gray new.txt -Bafg
		gmt plot t.txt -W1p
		gmt plot new.txt -Sc8p -Ggreen
		gmt plot t.txt -Sc8p -Gred
		gmt subplot set 2 -A"Cartesian: 1/0.5"
		gmt spatial t.txt -W1.5/0.5 > new.txt
		gmt plot -JX? -W4p,gray new.txt -Bafg
		gmt plot t.txt -W1p
		gmt plot new.txt -Sc8p -Ggreen
		gmt plot t.txt -Sc8p -Gred
		gmt subplot set 3 -A"Cartesian: 0.5 at end"
		gmt spatial t.txt -W0.5+l > new.txt
		gmt plot -JX? -W4p,gray new.txt -Bafg
		gmt plot t.txt -W1p
		gmt plot new.txt -Sc8p -Ggreen
		gmt plot t.txt -Sc8p -Gred
	gmt subplot end
gmt end show
