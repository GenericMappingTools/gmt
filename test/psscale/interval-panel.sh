#!/usr/bin/env bash
#
# Testing categorical colorbars with default panels and interval annotations
# E.g., https://forum.generic-mapping-tools.org/t/colorbar-box-and-annotation/4548

ps=interval-panel.ps

cat << EOF > t.cpt
# COLOR_MODEL = RGB
0	247/251/255	10	247/251/255
10	222/235/247	20	222/235/247
20	198/219/239	50	198/219/239
50	158/202/225	100	158/202/225
100	107/174/214	150	107/174/214
150	066/146/198	200	066/146/198
200	033/113/181	300	033/113/181
300	008/081/156	500	008/081/156
500	008/048/107	600	008/048/107
EOF
# Vertical colorbars
gmt psscale -Ct.cpt -Dx0c/13c+w12c+v -Li3p -F+p2p -P -K > ${ps}
gmt psscale -Ct.cpt -Dx5c/13c+w12c+v -LI3p -F+p2p -O -K >> ${ps}
gmt psscale -Ct.cpt -Dx10c/13c+w12c+v -Li3p -O -K >> ${ps}
gmt psscale -Ct.cpt -Dx15c/13c+w12c+v -LI3p -O -K >> ${ps}
# Horizontal colorbars
gmt psscale -Ct.cpt -Dx0c/1c+w18c+h -Li3p -F+p2p -O -K -X-1c >> ${ps}
gmt psscale -Ct.cpt -Dx0c/4c+w18c+h -LI3p -F+p2p -O -K >> ${ps}
gmt psscale -Ct.cpt -Dx0c/7c+w18c+h -Li3p -O -K >> ${ps}
gmt psscale -Ct.cpt -Dx0c/10c+w18c+h -LI3p -O >> ${ps}
