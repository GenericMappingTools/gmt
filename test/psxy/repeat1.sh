#!/usr/bin/env bash
# Test various repeating/clip mode for periodic and straight boundary
function map { #  $1 = -N option, $2 is text, $3 is any -Y option
        gmt psxy -R -J t.txt -Ct.cpt -Sc -Bafg -BWsne -O -K --MAP_FRAME_TYPE=plain $1 $3
        gmt psxy -R -J -O -K t.txt -Gblack -Sc0.1c
	echo 180 0 $2 | gmt pstext -R -J -O -K -F+f16p -Gwhite >> $ps
}
ps=repeat1.ps
cat << EOF > t.txt
8	45	0.5	0.5i
356	-45	1.5	0.3i
5	0	2.5	0.1i
EOF
gmt makecpt -Cred,blue,green -T0/3/1 -N > t.cpt
gmt psxy -R0/360/-60/60 -JM5i -T -P -K -Xc > $ps
map "" Default >> $ps
map -N -N  -Y2.3i >> $ps
map -Nr -Nr -Y2.3i >> $ps
map -Nc -Nc -Y2.3i >> $ps
gmt psxy -R -J -O -T >> $ps
