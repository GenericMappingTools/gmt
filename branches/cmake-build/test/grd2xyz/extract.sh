#!/bin/sh
#	$Id$
# Testing grd2xyz with -R and -s

. ../functions.sh
header "Test grd2xyz for subset and NaN-reversal"

ps=extract.ps
Rp=-21/11/-21/21
Rg=-20/10/-20/20
# Create geographic grid with NaNs inside a circle and 1 outside
grdmath -R$Rg -I1 -fg 355 2 SDIST 10 GT 0 NAN = tmp.nc
# Draw all nodes as open circles
grd2xyz tmp.nc | psxy -R$Rp -JM6i -Sc0.4c -W0.25p -P -K -B10f5WSne -Xc > $ps
# Fill all non-NaN nodes as blue
grd2xyz tmp.nc -s | psxy -R$Rp -J -Sc0.3c -Gblue -O -K >> $ps
# Fill nodes green inside the selected sub region
grd2xyz -R353/368/-15/14 tmp.nc | psxy -R$Rp -J -Sc0.2c -Ggreen -O -K >> $ps
# Show NaN nodes as red
grd2xyz tmp.nc -sr | psxy -R$Rp -J -Sc0.1c -Gred -O -K >> $ps
psxy -R$Rp -J -O -T >> $ps
pscmp
rm -f tmp.nc
