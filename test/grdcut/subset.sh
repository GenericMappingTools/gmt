#!/bin/sh
#	$Id: subset.sh,v 1.1 2011-06-03 20:47:15 guru Exp $
# Testing grdcut -Z

. ../functions.sh
header "Test grdcut for subset extraction based on data range"

ps=subset.ps
Rp=-21/21/-21/21
Rg=-20/20/-20/20
# Create  grid by evaluating a Kelvin-Bessel * sqrt(r) function
grdmath -R$Rg -I1 0 0 CDIST 0.1 MUL KEI 0 0 CDIST 0.1 MUL SQRT ADD DUP UPPER DIV 10 MUL = tmp.nc
makecpt -Crainbow -T-6/10/1 > t.cpt
# Draw all nodes as open circles
grd2xyz tmp.nc | psxy -R$Rp -JX6i -Sc0.25c -Ct.cpt -P -K -B10f5WSne -Xc -Y1.5i > $ps
psscale -D3i/-0.4i/6i/0.15ih -O -K -Ct.cpt -E >> $ps
# Extract portion of grid with values less than 5
grdcut tmp.nc -Z0/5 -Gout.nc
grd2xyz out.nc | psxy -R -J -Sc0.25c -W0.5p -O -K -B10f5WSne:."Rectangular subset with z <= 5": >> $ps
psxy -R$Rp -J -O -T >> $ps
pscmp
rm -f tmp.nc out.nc t.cpt
