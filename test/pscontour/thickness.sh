#!/usr/bin/env bash
# Test pscontour on stereographic projection with TIN data
# Comes from forum message http://gmt.soest.hawaii.edu/boards/1/topics/5532?r=5548
# Due to hairline differences in many gridlines between Linux and macOS we need a
# higher rms threshold for this test to pass
# GRAPHICSMAGICK_RMS = 0.021
ps=thickness.ps
cat << EOF > t.cpt
0 255 247 236   500 255 247 236
500 254 232 00   1000 254 232 00
1000 253 212 158   1500 253 212 158
1500 253 187 132   2000 253 187 132
2000 252 141 089   2500 252 141 089
2500 239 101 072   3000 239 101 072
3000 215 048 031   3500 215 048 031
EOF
gmt pscontour ice.bm -JS321/90/71/5i -P -R302/57/355/82.25r -Ct.cpt -I -W+ -K -Gl-45/81/-45/58 -Xc > $ps
gmt pscoast -J -R -W -O -Bafg --MAP_ANNOT_OBLIQUE=lon_horizontal >> $ps
