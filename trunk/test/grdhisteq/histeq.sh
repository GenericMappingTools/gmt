#!/bin/sh
#	$Id$
# Testing grdhisteq

ps=histeq.ps

makecpt -Crainbow -T0/1700/100 -Z > t.cpt
makecpt -Crainbow -T0/15/1 > c.cpt
grdgradient ../grdfft/topo.nc -Nt1 -fg -A45 -Gitopo.nc
grdhisteq ../grdfft/topo.nc -Gout.nc -C16
grdimage ../grdfft/topo.nc -Iitopo.nc -Ct.cpt -JM3i -Y6i -K -P -B5WSne > $ps
echo "315 -10 Original" | pstext -R../grdfft/topo.nc -J -O -K -F+jTR+f14p -T -Gwhite -W1p -Dj0.1i >> $ps
grdimage out.nc -Cc.cpt -J -X3.5i -K -O -B5WSne >> $ps
echo "315 -10 Equalized" | pstext -R -J -O -K -F+jTR+f14p -T -Gwhite -W1p -Dj0.1i >> $ps
psscale -D0i/-0.4i/5i/0.15ih -O -K -Ct.cpt -B500/:m: -E+n >> $ps
grdhisteq ../grdfft/topo.nc -Gout.nc -N
makecpt -Crainbow -T-3/3/0.1 -Z > c.cpt
grdimage out.nc -Cc.cpt -J -X-3.5i -Y-4i -K -O -B5WSne >> $ps
echo "315 -10 Normalized" | pstext -R -J -O -K -F+jTR+f14p -T -Gwhite -W1p -Dj0.1i >> $ps
grdhisteq ../grdfft/topo.nc -Gout.nc -N
grdimage out.nc -Cc.cpt -J -X3.5i -K -O -B5WSne >> $ps
echo "315 -10 Quadratic" | pstext -R -J -O -K -F+jTR+f14p -T -Gwhite -W1p -Dj0.1i >> $ps
psscale -D0i/-0.4i/5i/0.15ih -O -Cc.cpt -B1/:z: -E+n >> $ps

