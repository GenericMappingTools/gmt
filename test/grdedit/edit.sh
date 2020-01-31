#!/usr/bin/env bash
# Testing gmt grdedit -R -N

ps=edit.ps

Rp=-21/21/-21/21
Rg=-20/20/-20/20
# Create grid by evaluating a Kelvin-Bessel * sqrt(r) function
gmt grdmath -R$Rg -I1 0 0 CDIST 0.1 MUL KEI 0 0 CDIST 0.1 MUL SQRT ADD DUP UPPER DIV 10 MUL = tmp.nc
gmt makecpt -Crainbow -T-6/10/1 > t.cpt
# Draw all nodes as open circles
gmt grd2xyz tmp.nc | gmt psxy -R$Rp -JX4i -Ss0.2c -Ct.cpt -P -K -B10f5 -BWSne -Xc -Y1i > $ps
gmt psscale -Dx4.5i/4.25i+w6i/0.15i+jML+e -O -K -Ct.cpt >> $ps
# Change -R to 0/40/0/40 and replace the nodes in 10/20/10/20 with random values
Rp=-1/41/-1/41
gmt grdedit tmp.nc -R0/40/0/40
gmt grdcut tmp.nc -R10/20/10/20 -Gt.nc
gmt grdmath t.nc 0 MUL 10 ADD = t.nc
gmt grd2xyz t.nc > tmp
gmt grdedit tmp.nc -Ntmp
gmt grd2xyz tmp.nc | gmt psxy -R$Rp -JX4i -Ss0.2c -Ct.cpt -O -K -B10f5 -BWSne -Y4.5i >> $ps
gmt psxy -R$Rp -J -O -T >> $ps

