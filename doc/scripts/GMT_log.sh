#!/usr/bin/env bash
gmt begin GMT_log
	gmt set GMT_THEME cookbook
	gmt math -T0/100/1 T SQRT = sqrt.txt
	gmt math -T0/100/10 T SQRT = sqrt10.txt
	gmt plot -R1/100/0/10 -Jx4cl/0.4c -Bx2g3 -Bya2f1g2 -BWSne+gbisque -Wthick,blue,- -h sqrt.txt
	gmt plot -Ss0.3c -N -Gred -W -h sqrt10.txt
gmt end show
