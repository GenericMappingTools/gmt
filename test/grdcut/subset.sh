#!/bin/bash
#	$Id$
# Testing gmt grdcut -Z

ps=subset.ps

Rp=-21/21/-21/21
Rg=-20/20/-20/20
# Create  grid by evaluating a Kelvin-Bessel * sqrt(r) function
gmt grdmath -R$Rg -I1 0 0 CDIST 0.1 MUL KEI 0 0 CDIST 0.1 MUL SQRT ADD DUP UPPER DIV 10 MUL = tmp.nc
gmt makecpt -Crainbow -T-6/10/1 > t.cpt
# Draw all nodes as open circles
gmt grd2xyz tmp.nc | gmt psxy -R$Rp -JX6i -Sc0.25c -Ct.cpt -P -K -B10f5 -BWSne -Xc -Y1.5i > $ps
gmt psscale -D3i/-0.4i/6i/0.15ih -O -K -Ct.cpt -E >> $ps
# Extract portion of grid with values less than 5
gmt grdcut tmp.nc -Z0/5 -Gout.nc
gmt grd2xyz out.nc | gmt psxy -R -J -Sc0.25c -W0.5p -O -K -B10f5 -BWSne+t"Rectangular subset with z <= 5" >> $ps
gmt psxy -R$Rp -J -O -T >> $ps

