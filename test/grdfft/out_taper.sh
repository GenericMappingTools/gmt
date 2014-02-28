#!/bin/bash
#	$Id$
# Testing gmt grdfft -N+m mirror reflection and -N+e edge-point symmetry

ps=out_taper.ps

# Create a egg-carton input grid
gmt grdmath -R0/300/0/200 -I1 X 5 MUL COSD Y 10 MUL SIND MUL = t.nc
gmt grdfft t.nc -N512/384+m+t50+wtmp1+l -E > /dev/null
gmt grdfft t.nc -N512/384+e+wtmp2+l -E > /dev/null
cat << EOF > box
0	0
300	0
300	200
0	200
EOF

scl=0.008
x=`gmt gmtmath -Q 512 2 DIV $scl MUL =`
xoff=`gmt gmtmath -Q 512 300 SUB 2 DIV $scl MUL NEG =`
yoff=`gmt gmtmath -Q 200 $scl MUL 0.5 ADD =`
yoffe=`gmt gmtmath -Q 384 $scl MUL 0.5 ADD =`
gmt makecpt -Cpolar -T-1/1/0.1 -Z > t.cpt
gmt grdimage t.nc -Jx${scl}i -Ct.cpt -P -Ba -BWSne -K > $ps
gmt grd2xyz t_tmp2.nc | awk '{if ($2 == 100) print $1, $3}' > tmp
R=`gmt info tmp -I10/3`
gmt psxy $R -JX3.5i/1.6i -O -K -W1p,green -Bxaf -Byafg10 -BWSne -X3i tmp >> $ps
gmt psxy -R -J -O -K -W0.5p,- << EOF >> $ps
>
0	-3
0	3
>
300	-3
300	3
EOF
# mirror and taper
gmt grdimage t_tmp1.nc -Jx${scl}i -Ct.cpt -Ba -BWSne -O -K -X-3i -Y${yoff}i >> $ps
gmt psxy -Rt_tmp1.nc -J -O -K -L -W2p box >> $ps
echo "400 192 Mirror symmetry" | gmt pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps
echo "400 192 50% outward taper" | gmt pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/-0.3i >> $ps
# edge-symmetry and taper
gmt grdimage t_tmp2.nc -J -Ct.cpt -Ba -BWSne -O -K -Y${yoffe}i >> $ps
gmt psxy -Rt_tmp2.nc -J -O -K -L -W2p box >> $ps
gmt psxy -R -J -O -K -W2p,green << EOF >> $ps
-105	100
410	100
EOF
echo "400 192 Point symmetry" | gmt pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps
echo "400 192 100% outwardtaper" | gmt pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/-0.3i >> $ps
gmt psscale -Ct.cpt -D${x}i/${yoffe}i/4i/0.1ih -O -K -B0.5 >> $ps
gmt psxy -R -J -O -T >> $ps
