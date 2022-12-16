#!/usr/bin/env bash
# test the -Ar|t line-resampling for polar projections
cat << EOF > t.txt
10  5
30 4
50 7
EOF
gmt begin polarcurves ps
	gmt subplot begin 3x1 -R0/60/0/10 -Fs8c -JP8c -A
		gmt subplot set 0 -A"No -A"
		gmt plot -W2p,red t.txt
		gmt plot t.txt -Sc0.2c -Wthin
		gmt subplot set 1 -A"-At: @~q@~, then r"
		gmt plot -W2p,red -At t.txt
		gmt plot t.txt -Sc0.2c -Wthin
		gmt subplot set 2 -A"-Ar: r, then @~q@~"
		gmt plot -W2p,red -Ar t.txt
		gmt plot t.txt -Sc0.2c -Wthin
	gmt subplot end
gmt end show
