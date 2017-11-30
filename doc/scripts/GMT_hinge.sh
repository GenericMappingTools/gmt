#!/bin/bash
#	$Id$
#
ps=GMT_hinge.ps
gmt makecpt -Cglobe -T-8000/3000 > t.cpt
gmt psscale -Ct.cpt  -K -P -Baf -Dx0/0+w4.5i/0.1i+h -W0.001 > $ps
gmt psscale -Cglobe  -O -K -Baf -Dx0/0+w4.5i/0.1i+h -W0.001 -Y0.5i >> $ps
echo 2.25 0.1 90 0.2i | gmt psxy -R0/4.5/0/1 -Jx1i -O -K -Sv0.1i+a80+b -W1p -Gblack >> $ps
gmt pstext -R -J -O -F+f12p+jCB << EOF >> $ps
2.25	0.35	HINGE
EOF
