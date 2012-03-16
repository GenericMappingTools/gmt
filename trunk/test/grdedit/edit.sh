#!/bin/sh
#	$Id$
# Testing grdedit -R -N

header "Test grdedit to change region and replace node values"

Rp=-21/21/-21/21
Rg=-20/20/-20/20
# Create grid by evaluating a Kelvin-Bessel * sqrt(r) function
grdmath -R$Rg -I1 0 0 CDIST 0.1 MUL KEI 0 0 CDIST 0.1 MUL SQRT ADD DUP UPPER DIV 10 MUL = tmp.nc
makecpt -Crainbow -T-6/10/1 > t.cpt
# Draw all nodes as open circles
grd2xyz tmp.nc | psxy -R$Rp -JX4i -Ss0.2c -Ct.cpt -P -K -B10f5WSne -Xc -Y1i > $ps
psscale -D4.5i/4.25i/6i/0.15i -O -K -Ct.cpt -E >> $ps
# Change -R to 0/40/0/40 and replace the nodes in 10/20/10/20 with random values
Rp=-1/41/-1/41
grdedit tmp.nc -R0/40/0/40
grdcut tmp.nc -R10/20/10/20 -Gt.nc
grdmath t.nc 0 MUL 10 ADD = t.nc
grd2xyz t.nc > tmp
grdedit tmp.nc -Ntmp
grd2xyz tmp.nc | psxy -R$Rp -JX4i -Ss0.2c -Ct.cpt -O -K -B10f5WSne -Y4.5i >> $ps
psxy -R$Rp -J -O -T >> $ps

pscmp
