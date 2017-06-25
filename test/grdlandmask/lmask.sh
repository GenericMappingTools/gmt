#!/bin/bash
# Test grdlandmask on ocean/land/lake
ps=lmask.ps
cat << EOF > t.cpt
1	blue
2	brown
3	cyan
EOF
gmt pscoast -R9:30/10/37/37:30 -JX4.5id -Baf -BWSne -Di -Gred -K -P -Xc -Y0.75i > $ps
gmt grdlandmask -R -Di -r -I1m -N1/2/3/4/5 -Gt.nc
gmt grdimage t.nc -J -O -K -Ct.cpt -Bafg1m -BWSne -Y5.1i >> $ps
gmt pscoast -R -J -Di -Wthin -O >> $ps
