#!/bin/sh
#	$Id$
# Testing grdfft -N+m mirror reflection and -N+e edge-point symmetry

ps=out_taper.ps

# Create a egg-carton input grid
grdmath -R0/300/0/200 -I1 X 5 MUL COSD Y 10 MUL SIND MUL = t.nc
grdfft t.nc -N512/384+m+t50 -Qtmp1 -L -E > /dev/null
grdfft t.nc -N512/384+e -Qtmp2 -L -E > /dev/null
cat << EOF > box
0	0
300	0
300	200
0	200
EOF

scl=0.008
x=`gmtmath -Q 512 2 DIV $scl MUL =`
xoff=`gmtmath -Q 512 300 SUB 2 DIV $scl MUL NEG =`
yoff=`gmtmath -Q 200 $scl MUL 0.5 ADD =`
yoffe=`gmtmath -Q 384 $scl MUL 0.5 ADD =`
makecpt -Cpolar -T-1/1/0.1 -Z > t.cpt
grdimage t.nc -Jx${scl}i -Ct.cpt -P -BaWSne -K > $ps
grd2xyz tmp2_t.nc | awk '{if ($2 == 100) print $1, $3}' > tmp
R=`minmax tmp -I10/3`
psxy $R -JX3.5i/1.6i -O -K -W1p,green -Baf/afg10WSne -X3i tmp >> $ps
psxy -R -J -O -K -W0.5p,- << EOF >> $ps
>
0	-3
0	3
>
300	-3
300	3
EOF
# mirror and taper
grdimage tmp1_t.nc -Jx${scl}i -Ct.cpt -BaWSne -O -K -X-3i -Y${yoff}i >> $ps
psxy -Rtmp1_t.nc -J -O -K -L -W2p box >> $ps
echo "400 192 Mirror symmetry" | pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps
echo "400 192 50% outward taper" | pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/-0.3i >> $ps
# edge-symmetry and taper
grdimage tmp2_t.nc -J -Ct.cpt -BaWSne -O -K -Y${yoffe}i >> $ps
psxy -Rtmp2_t.nc -J -O -K -L -W2p box >> $ps
psxy -R -J -O -K -W2p,green << EOF >> $ps
-105	100
410	100
EOF
echo "400 192 Point symmetry" | pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps
echo "400 192 100% outwardtaper" | pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/-0.3i >> $ps
psscale -Ct.cpt -D${x}i/${yoffe}i/4i/0.1ih -O -K -B0.5 >> $ps
psxy -R -J -O -T >> $ps
