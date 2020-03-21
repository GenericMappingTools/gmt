#!/usr/bin/env bash
# Testing gmt grdfft

ps=gfilter.ps
gmt set GMT_FFT kiss

topo=@topo_38.nc

# Do a 100 km Gaussian filter on some topography
gmt grdfft ${topo} -fg -F-/100000 -Glow.nc -N+l
gmt makecpt -Crainbow -T0/1700 > t.cpt
gmt grdgradient ${topo} -Nt1 -fg -A45 -Gitopo.nc
gmt grdgradient low.nc -Nt1 -fg -A45 -Gilow.nc
gmt grdimage ${topo} -Iitopo.nc -Ct.cpt -JM6i -Y6i -Xc -K -P -B5 -BWSne > $ps
echo "315 -10 Original" | gmt pstext -R${topo} -J -O -K -F+jTR+f14p -C+t -Gwhite -W1p -Dj0.1i >> $ps
gmt grdimage low.nc -Iilow.nc -Ct.cpt -J -Y-4.6i -K -O -B5 -BWSne >> $ps
echo "315 -10 100 km Gaussian" | gmt pstext -R -J -O -K -F+jTR+f14p -C+t -Gwhite -W1p -Dj0.1i >> $ps
gmt psscale -Dx3i/-0.4i+w5i/0.15i+h+jTC+e+n -O -Ct.cpt -B500 >> $ps

