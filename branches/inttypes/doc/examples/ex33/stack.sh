#!/bin/sh
#	$Id$
# Testing grdtrack -C -S stacking

ps=stack.ps

# grdcut etopo1m_grd.nc -R118W/107W/49S/42S -Gspac.nc
makecpt -Crainbow -T-5000/-2000/500 -Z > z.cpt
grdgradient spac.nc -A15 -Ne0.75 -Gspac_int.nc
grdimage spac.nc -Ispac_int.nc -Cz.cpt -JM6i -P -Baf -K -Xc --FORMAT_GEO_MAP=dddF > $ps
cat << EOF > ridge.txt
-111.6	-43
-113.3	-47.5
EOF
psxy -Rspac.nc -J -O -K -W2p,blue ridge.txt >> $ps
psxy -R -J -O -K -Sc0.1i -Gblue ridge.txt >> $ps
grdtrack ridge.txt -Gspac.nc -C400k/2k/10k -Se+s > table.txt
psxy -R -J -O -K -W0.5p table.txt >> $ps
gmtconvert stacked_profile.txt -o0,5 > env.txt
gmtconvert stacked_profile.txt -o0,6 -I -T >> env.txt

psxy -R-200/200/-3500/-2000 -JX6i/3i -Bafg1000:"Distance from ridge (km)":/af:"Depth (m)":WSne -O -K -Glightgray env.txt -Y6.5i >> $ps
psxy -R -J -O -K -W3p stacked_profile.txt >> $ps
echo "0 -2000 MEDIAN STACKED PROFILE" | pstext -R -J -O -K -Gwhite -F+jTC+f14p -Dj0.1i/0.1i >> $ps
psxy -R -J -O -T >> $ps
gv $ps &
rm -f z.cpt spac_int.nc ridge.txt table.txt env.txt stacked_profile.txt
