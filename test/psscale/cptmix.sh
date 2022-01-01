#!/usr/bin/env bash
# See issue # 1093.  Fixed in r18328
ps=cptmix.ps
cat << EOF > ex16_v9.cpt
30 p200/16 80 -
80 - 100 -
100 200/0/0 200 255/255/0
200 yellow 300 green
EOF
cat << EOF > ex16_v10.cpt
30 p200/16 80 -
80 - 100 -
100 255/255/0 200 255/255/0
200 green 300 green
EOF
cat << EOF > ex16_v11.cpt
80 	- 	 	100 		-
100 	200/0/0 	200		255/255/0
200 	yellow 		300 		green
EOF

gmt psxy -R-72/-64/-35/-30 -JM14c -T -K -P -X1.5i > $ps
gmt psscale -R -J -O -K -Cex16_v9.cpt >> $ps -DJLM+w8.5c/0.618c+e+n -Ba+l"v9" -X3c
gmt psscale -R -J -O -K -Cex16_v10.cpt >> $ps -DJLM+w8.5c/0.618c+e+n -Ba+l"v10" -X5c
gmt psscale -R -J -O -K -Cex16_v11.cpt >> $ps -DJLM+w8.5c/0.618c+e+n -Ba+l"v11" -X5c
gmt psscale -R -J -O -K -Cex16_v9.cpt  >> $ps -DJLM+w8.5c/0.618c+e+n -I -Ba+l"v9" -X-10c -Y12c
gmt psscale -R -J -O -K -Cex16_v10.cpt >> $ps -DJLM+w8.5c/0.618c+e+n -I -Ba+l"v10" -X5c
gmt psscale -R -J -O -Cex16_v11.cpt >> $ps -DJLM+w8.5c/0.618c+e+n -I -Ba+l"v11" -X5c
