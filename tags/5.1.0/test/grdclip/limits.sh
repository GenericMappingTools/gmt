#!/bin/bash
#	$Id$
# Testing gmt grdclip

ps=limits.ps

Rp=-21/21/-21/21
Rg=-20/20/-20/20
# Create  grid by evaluating a Kelvin-Bessel * sqrt(r) function
gmt grdmath -R$Rg -I1 0 0 CDIST 0.1 MUL KEI 0 0 CDIST 0.1 MUL SQRT ADD DUP UPPER DIV 10 MUL = tmp.nc
gmt makecpt -Crainbow -T-6/10/1 > t.cpt
# Draw all nodes as open circles
gmt grd2xyz tmp.nc | gmt psxy -R$Rp -JX3.25i -Sc0.15c -W0.25p -Ct.cpt -P -K -B10f5 -BWSne -Xc -Y0.5i > $ps
gmt psscale -D4i/4.75i/6i/0.15i -O -K -Ct.cpt -E+n >> $ps
# Set values higher than 7 to NaN and values lower than 0 to -6
gmt grdclip tmp.nc -Sa7/NaN -Sb0/-6 -Gout.nc
gmt grd2xyz out.nc | gmt psxy -R$Rp -J -Sc0.15c -W0.25p -Ct.cpt -O -K -B10f5 -BWsne -Y3.5i >> $ps
# Set values between 0 and 5 to NaN
gmt grdclip tmp.nc -Si0/5/NaN -Gout.nc
gmt grd2xyz out.nc | gmt psxy -R$Rp -J -Sc0.15c -W0.25p -Ct.cpt -O -K -B10f5 -BWsne -Y3.5i >> $ps
gmt psxy -R$Rp -J -T -O >> $ps
