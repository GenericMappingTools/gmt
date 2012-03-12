#!/bin/sh
#	$Id$
# Testing grdclip

. functions.sh
header "Test grdclip for NaN and value clipping"

ps=limits.ps
Rp=-21/21/-21/21
Rg=-20/20/-20/20
# Create  grid by evaluating a Kelvin-Bessel * sqrt(r) function
grdmath -R$Rg -I1 0 0 CDIST 0.1 MUL KEI 0 0 CDIST 0.1 MUL SQRT ADD DUP UPPER DIV 10 MUL = tmp.nc
makecpt -Crainbow -T-6/10/1 > t.cpt
# Draw all nodes as open circles
grd2xyz tmp.nc | psxy -R$Rp -JX4.5i -Sc0.25c -W0.25p -Ct.cpt -P -K -B10f5WSne -Xc -Y0.5i > $ps
psscale -D5i/4.75i/6i/0.15i -O -K -Ct.cpt -E+n >> $ps
# Set values higher than 7 to NaN and values lower than 0 to -6
grdclip tmp.nc -Sa7/NaN -Sb0/-6 -Gout.nc
grd2xyz out.nc | psxy -R -J -Sc0.25c -W0.25p -Ct.cpt -O -K -B10f5WSne -Y5i >> $ps
psxy -R$Rp -J -O -T >> $ps

pscmp
