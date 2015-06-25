#!/bin/bash
ps=mapscales.ps
gmt set FONT_LABEL 9p PROJ_ELLIPSOID sphere
# Use sphere so we can compute scale between longitude and km exactly
R=6371.0087714
# Determine degree longitude at Equator that equals 5000 km
dlon=`gmt math -Q 5000 $R DIV PI DIV 180 MUL 2 DIV =`
gmt psxy -R-100/100/0/80 -JM6i -P -Baf -K -Xc -W0.25p,- << EOF > $ps
>
-$dlon	0
-$dlon	80
>
$dlon	0
$dlon	80
EOF
for lat in 76 70 60 45 30 10; do
	gmt psbasemap -R -J -Lf0/$lat/0/5000k+l"5000 km at Equator" -O -K >> $ps
done
gmt psxy -R -J -O -Baf -K -Y5i -W0.25p,- << EOF >> $ps
>
-$dlon	0
-$dlon	80
>
$dlon	0
$dlon	80
EOF
# Plot exact map scale with as function of latitude
gmt math -T0/80/1 T COSD INV $dlon MUL = t.txt
gmt psxy -R -J -O -K -: t.txt -W0.25p >> $ps
gmt math -T0/80/1 T COSD INV $dlon MUL NEG = t.txt
gmt psxy -R -J -O -K -: t.txt -W0.25p >> $ps
for lat in 76 70 60 45 30 10; do
	gmt psbasemap -R -J -Lf0/$lat/$lat/5000k+l"5000 km at ${lat}\312N" -O -K >> $ps
done
gmt psxy -R -J -O -T >> $ps
