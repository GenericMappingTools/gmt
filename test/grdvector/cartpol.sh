#!/usr/bin/env bash
# Testing grdvector for Cartesian and polar projections
# Added to address issue #767

ps=cartpol.ps
cat << EOF > vel_x
1
1
1
1
1
1
1
1
1
EOF
cat << EOF > vel_y
0
0
0
1
1
1
0
0
0
EOF

for i in vel_x vel_y; do
    gmt xyz2grd $i -G$i.nc -R0/90/0.5/1 -I45/0.25 -ZLB
done
gmt grd2xyz vel_x.nc > a
gmt grd2xyz vel_y.nc > b

# Cartesian
gmt grdvector vel_x.nc vel_y.nc	-JX6i/3i  -Baf -BWSne -Q0.3i+e -S2i -N -W1p,red -Gred -P -K > $ps
paste a b | awk '{printf "%s %s (%s,%s)\n", $1, $2, $3, $6}' | gmt pstext -R -J -O -K -N -D0.5i/0.1i -F+f12p+jLB >> $ps
# Polar
gmt grdvector vel_x.nc vel_y.nc	-JP6i+a+t45 -Baf -BWSne -Q0.3i+e -S2i -N -W1p,red -Gred -T -O -K -Y5i >> $ps
paste a b | awk '{printf "%s %s (%s,%s)\n", $1, $2, $3, $6}' | gmt pstext -R -J -O -N -D-0.2i/0.3i -F+f12p+jLB >> $ps
