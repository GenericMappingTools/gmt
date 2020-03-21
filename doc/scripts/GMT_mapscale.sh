#!/usr/bin/env bash
gmt begin GMT_mapscale
	gmt basemap -R0/40/50/56 -JM5i -B -LjML+c53+w1000k+f+l"Scale at 53@.N" -F+glightcyan+c0+p
	gmt basemap -LjBR+c53+w1000k+l+f -F+p1p+i+gwhite+c0.1i
h=$(gmt mapproject -Wh -Di)
h=$(gmt math -Q $h 2 DIV =)
lat=$(echo 0 $h | gmt mapproject -I -Di -o1)
	gmt plot -Wfaint -A -N << EOF
>
0	$lat
40	$lat
>
0	50
40	50
EOF
gmt end show
