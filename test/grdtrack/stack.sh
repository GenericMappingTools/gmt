#!/bin/sh
#	$Id$
# Testing grdtrack -C -S stacking

ps=stack.ps

# grdcut etopo2m_grd.nc -R118W/107W/49S/42S -Gspac.nc
makecpt -Crainbow -T-5000/-2000/500 -Z > z.cpt
grdimage spac.nc -Cz.cpt -JM6i -P -B2 -K > $ps
cat << EOF > ridge.txt
-111.6	-43
-113.3	-47.5
EOF
psxy -Rspac.nc -J -O -K -W1p ridge.txt >> $ps
#grdtrack ridge.txt -Gspac.nc -C400k/2k/10k -V -Se+s > table.txt
grdtrack ridge.txt -Gspac.nc -C400k/2k/10k -V > table.txt
psxy -R -J -O -K -W1p table.txt >> $ps
psxy -R -J -O -T >> $ps
gv $ps &
