#!/usr/bin/env bash
#
# Testing gmt pslegend modern mode mixing of hidden and explicit legend info

cat << EOF > legend.txt
S - kvolcano 0.3c red - - Plot1
S - kvolcano 0.3c green - - Plot2
EOF
gmt begin mixlegend png
	gmt subplot begin 3x2 -R0/10/0/10 -Fs8c -A+jTR
		gmt subplot set -A"No -M"
		echo 2 2 | gmt plot -Sc0.2c -Gblack -lLABEL1
		echo 5 5 | gmt plot -St0.2c -Gred -lLABEL2
		gmt legend legend.txt -DjTL -F
		gmt subplot set -A"-M"
		echo 2 2 | gmt plot -Sc0.2c -Gblack -lLABEL1
		echo 5 5 | gmt plot -St0.2c -Gred -lLABEL2
		gmt legend legend.txt -DjTL -F -M
		gmt subplot set -A"-Mh"
		echo 2 2 | gmt plot -Sc0.2c -Gblack -lLABEL1
		echo 5 5 | gmt plot -St0.2c -Gred -lLABEL2
		gmt legend legend.txt -DjTL -F -Mh
		gmt subplot set -A"-Me"
		echo 2 2 | gmt plot -Sc0.2c -Gblack -lLABEL1
		echo 5 5 | gmt plot -St0.2c -Gred -lLABEL2
		gmt legend legend.txt -DjTL -F -Me
		gmt subplot set -A"-Meh"
		echo 2 2 | gmt plot -Sc0.2c -Gblack -lLABEL1
		echo 5 5 | gmt plot -St0.2c -Gred -lLABEL2
		gmt legend legend.txt -DjTL -F -Meh
		gmt subplot set -A"-Mhe"
		echo 2 2 | gmt plot -Sc0.2c -Gblack -lLABEL1
		echo 5 5 | gmt plot -St0.2c -Gred -lLABEL2
		gmt legend legend.txt -DjTL -F -Mhe
	gmt subplot end
gmt end show
