#!/bin/bash
#	$Id$
# Testing gmt grdsample

ps=sample.ps

Rg=-20/20/-20/20
# Create  grid by evaluating a Kelvin-Bessel * sqrt(r) function
gmt grdmath -R$Rg -I1 0 0 CDIST 0.1 MUL KEI 0 0 CDIST 0.1 MUL SQRT ADD DUP UPPER DIV 10 MUL = tmp.nc
gmt makecpt -Crainbow -T-6/10/1 > t.cpt
# Draw all nodes as open circles
gmt grdimage tmp.nc -JX4.5i -Ct.cpt -P -K -B10f5 -BWSne -Xc -Y0.75i > $ps
gmt psscale -D5i/4.75i+w6i/0.15i+jML+e+n -O -K -Ct.cpt >> $ps
# Resample to 0.2 spacing
gmt grdsample tmp.nc -I0.2 -Gout.nc
gmt grdimage out.nc -JX4.5i -Ct.cpt -O -K -B10f5 -BWSne -Xc -Y5i >> $ps
gmt psxy -R$Rp -J -O -T >> $ps

