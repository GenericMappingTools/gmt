#!/usr/bin/env bash
# Test auto-legend for histogram and add in a single plot call as well
gmt begin histlegend ps
	gmt convert hist.txt -i0+o-1.5 > shift.txt
	gmt histogram -R2/7/0/10 hist.txt -T0.2 -B -BWSrt -Gred@50 -W0.5p -l"Grapefruit"
	gmt histogram shift.txt -T0.2 -B -Ggreen@50 -W0.5p -l"Kiwifruit"
	echo 3.7 9.25 | gmt plot -Si0.5c -Gorange -W0.25p -l"Maximum"
gmt end show
