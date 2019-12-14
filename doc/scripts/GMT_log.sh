#!/usr/bin/env bash
gmt begin GMT_log
	gmt plot -R1/100/0/10 -Jx1.5il/0.15i -Bx2g3 -Bya2f1g2 -BWSne+gbisque -Wthick,blue,- -h sqrt.txt
	gmt plot -Ss0.1i -N -Gred -W -h sqrt10.txt
gmt end show
