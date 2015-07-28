#!/bin/bash
#	$Id$
# Testing gmt grdfft -N+n with 10% and 25 % tapering

ps=in_taper.ps

# Create a egg-carton input grid
gmt grdmath -R0/300/0/200 -I1 -r X 5 MUL COSD Y 10 MUL SIND MUL = t.nc
cat << EOF > box
0	0
300	0
300	200
0	200
EOF

scl=0.01
x=`gmt gmtmath -Q 512 2 DIV $scl MUL =`
xoff=`gmt gmtmath -Q 512 300 SUB 2 DIV $scl MUL NEG =`
yoff=`gmt gmtmath -Q 200 $scl MUL 0.5 ADD =`
yoffe=`gmt gmtmath -Q 384 $scl MUL 0.5 ADD =`
gmt makecpt -Cpolar -T-1/1/0.1 -Z > t.cpt
gmt grdimage t.nc -Jx${scl}i -Ct.cpt -P -Ba -BWSne -K -X1.75i > $ps
echo "350 100 Original Data" | gmt pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps

# mirror and taper
gmt grdfft t.nc -N512/384+n+t10+wtmp+l -E > /dev/null
gmt grdimage t_tmp.nc -J -Ct.cpt -Ba -BWSne -O -K -Y${yoff}i -X${xoff}i >> $ps
gmt psxy -Rt_tmp.nc -J -O -K -L -W2p box >> $ps
echo "400 192 Extended" | gmt pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps
echo "400 192 10% inward taper" | gmt pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/-0.3i >> $ps
# edge-symmetry and taper
gmt grdfft t.nc -N300/200+n+t25+wtmp+l -E > /dev/null
gmt grdimage t_tmp.nc -J -Ct.cpt -Ba -BWSne -O -K -Y${yoffe}i >> $ps
gmt psxy -Rt_tmp.nc -J -O -K -L -W2p box >> $ps
echo "400 192 No extension" | gmt pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps
echo "400 192 25% inward taper" | gmt pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/-0.3i >> $ps
gmt psscale -Ct.cpt -D${x}i/${yoff}i+w4i/0.1i+h+jTC -O -K -B0.5 >> $ps
gmt psxy -R -J -O -T >> $ps
