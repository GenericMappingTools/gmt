#!/bin/bash
#	$Id$
# Testing gmt grd2xyz with -R and -s

ps=extract.ps

Rp=-21/11/-21/21
Rg=-20/10/-20/20
gmt set PROJ_ELLIPSOID Sphere
# Create geographic grid with NaNs inside a circle and 1 outside
gmt grdmath -R$Rg -I1 -fg 355 2 SDIST 10 DEG2KM GT 0 NAN = tmp.nc
# Draw all nodes as open circles
gmt grd2xyz tmp.nc | gmt psxy -R$Rp -JM6i -Sc0.4c -W0.25p -P -K -B10f5 -BWSne -Xc > $ps
# Fill all non-NaN nodes as blue
gmt grd2xyz tmp.nc -s | gmt psxy -R$Rp -JM6i -Sc0.3c -Gblue -O -K >> $ps
# Fill nodes green inside the selected sub region
gmt grd2xyz -R353/368/-15/14 tmp.nc | gmt psxy -R$Rp -JM6i -Sc0.2c -Ggreen -O -K >> $ps
# Show NaN nodes as red
gmt grd2xyz tmp.nc -sr | gmt psxy -R$Rp -JM6i -Sc0.1c -Gred -O -K >> $ps
gmt psxy -R$Rp -J -O -T >> $ps
