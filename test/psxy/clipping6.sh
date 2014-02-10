#!/bin/bash
ps=clipping6.ps
cat << EOF > badpol.txt
-70	-55
-50	-30
-50	50
-120	67
-170	60
-80	0
EOF
gmt psxy -R-90/270/-60/75 -JM5i -A -Ggreen -W0.25p,red badpol.txt -Bag -K -P -Xc > $ps
gmt psxy -R-180/180/-60/75 -JR90/5i -A -Ggreen -W0.25p,red badpol.txt -Bag -O -K -Y3.5i >> $ps
gmt psxy -R -JR-90/5i -A -Ggreen -W0.25p,red badpol.txt -Bag -O -Y3.5i >> $ps
