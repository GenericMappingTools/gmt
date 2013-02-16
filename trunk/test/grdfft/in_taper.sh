#!/bin/sh
#	$Id$
# Testing grdfft -N+n with 10% and 25 % tapering

ps=in_taper.ps

# Create a egg-carton input grid
grdmath -R0/300/0/200 -I1 -r X 5 MUL COSD Y 10 MUL SIND MUL = t.nc
cat << EOF > box
0	0
300	0
300	200
0	200
EOF

scl=0.01
x=`gmtmath -Q 512 2 DIV $scl MUL =`
xoff=`gmtmath -Q 512 300 SUB 2 DIV $scl MUL NEG =`
yoff=`gmtmath -Q 200 $scl MUL 0.5 ADD =`
yoffe=`gmtmath -Q 384 $scl MUL 0.5 ADD =`
makecpt -Cpolar -T-1/1/0.1 -Z > t.cpt
grdimage t.nc -Jx${scl}i -Ct.cpt -P -BaWSne -K -X1.75i > $ps
echo "350 100 Original Data" | pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps

# mirror and taper
grdfft t.nc -N512/384+n+t10+wtmp -L -E > /dev/null
grdimage t_tmp.nc -J -Ct.cpt -BaWSne -O -K -Y${yoff}i -X${xoff}i >> $ps
psxy -Rt_tmp.nc -J -O -K -L -W2p box >> $ps
echo "400 192 Extended" | pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps
echo "400 192 10% inward taper" | pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/-0.3i >> $ps
# edge-symmetry and taper
grdfft t.nc -N300/200+n+t25+qtmp -L -E > /dev/null
grdimage t_tmp.nc -J -Ct.cpt -BaWSne -O -K -Y${yoffe}i >> $ps
psxy -Rt_tmp.nc -J -O -K -L -W2p box >> $ps
echo "400 192 No extension" | pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/0 >> $ps
echo "400 192 25% inward taper" | pstext -R -J -O -K -N -F+jLM+f16p -D0.5i/-0.3i >> $ps
psscale -Ct.cpt -D${x}i/${yoff}i/4i/0.1ih -O -K -B0.5 >> $ps
psxy -R -J -O -T >> $ps
