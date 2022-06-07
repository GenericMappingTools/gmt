#!/usr/bin/env bash
# Test that plain-headed geovectors work

gmt begin geovecplain
	echo -0.2 0.25 45 100 | gmt plot -R-0.25/1/-0.25/1 -JM15c -S=32p+eA+bA -W2p -B --MAP_FRAME_TYPE=plain
	echo 0 0.25 45 100 | gmt plot -S=32p+eAl+bAr -W2p
	echo 0 -0.2 45 100 | gmt plot -S=32p+eal+bar -Gred -W2p
	echo 0.2 -0.2 45 100 | gmt plot -S=32p+ea+ba -Gred -W2p
gmt end show
