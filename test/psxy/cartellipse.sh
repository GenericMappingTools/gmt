#!/usr/bin/env bash
# Test Cartesian plotting of ellipses

gmt begin cartellipse ps
	gmt subplot begin 2x2 -Fs7c
		echo "0 0 0.1" | gmt plot -JX7c/7c -R-0.1/0.1/-1/1 -SE- -Gred -Wthinner,black -c -BWNrb
		echo "0 0 0.1" | gmt plot -JX7c/7c -R-1/1/-1/1 -SE- -Gred -Wthinner,black -c -BlbNE
        echo "0 0 0.1" | gmt plot -JX7c/7c -R-1/1/-0.1/0.1 -SE- -Gred -Wthinner,black -c -BWStr
        echo "0 -0.1 0.1" | gmt plot -JX7c/7c -R-1/1/-0.1/0.1 -SE- -Gred -Wthinner,black -c -BlStE
	gmt subplot end
gmt end show
