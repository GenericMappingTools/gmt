#!/usr/bin/env bash
#
# Test crosslines for contour annotation placements and scaling

ps=bigisland.ps
cat << EOF > crosslines.txt
> ML to the base
155.5922W	19.4721N
155:55W	19:00N
156:45W	19:00N
> MK to top right corner
155.4681W	19.8206N
154:30W		20:30N
EOF
gmt pscoast -R156:45W/154:15W/18:30N/20:30N -JM6i -Baf -Gburlywood -W0.75p -Df -P -K --FORMAT_GEO_MAP=dddF > $ps
gmt grdcontour -R @earth_relief_01m -J -Z+s0.001 -A1+f7p+a0  -Wc0.25p,gray -Wa0.75p,gray -O -K -L-7/-0.001 -Gxcrosslines.txt >> $ps
gmt grdcontour -R @earth_relief_01m -J -Z+s0.001 -A1+f7p+a0 -C0.5 -O -L0.001/5 -Gxcrosslines.txt >> $ps
