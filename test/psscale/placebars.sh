#!/usr/bin/env bash
ps=placebars.ps
gmt makecpt -Crainbow -T0/10 > t.cpt
gmt pscoast -R-180/180/-90/-60 -JS0/-90/4i -Bfg15 -Di -Glightgray -P -K -Xc -Yc --MAP_FRAME_TYPE=plain > $ps
for code in BL BC BR ML MC MR TL TC TR; do
	gmt psscale -Ct.cpt -DJ${code}+w1i/0.15i+o0.0i -R -J -O -K -Bx0 -By+l${code} >> $ps
done
gmt psbasemap -R0/4/0/4 -JX4i -B0 -O --MAP_FRAME_PEN=0.25p,- >> $ps
