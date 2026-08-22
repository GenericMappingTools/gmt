#!/usr/bin/env bash
ps=diagonal.ps
gmt psxy -R0/90/0/70 -JM6i -Wthick,green -Gcyan -Baf -BWSne+t"Rectangle via diagonal" -Sr+s -P -K -Xc << EOF > $ps
15	5	25	35
2	30	10	50
> -Gred
15	44	38	50
> -G- -W5p,black
25	55	50	65
EOF
gmt psxy -R -J -O -A -Wthin -Glightbrown -Sr+s << EOF >> $ps
50	35	75	5
42	50	60	30
> -Gred
88	50	65	65
EOF
