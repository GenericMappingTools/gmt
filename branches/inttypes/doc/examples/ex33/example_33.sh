#!/bin/bash
#               GMT EXAMPLE 33
#               $Id$
#
# Purpose:      Illustrate grdtracks new cross-track and stacking options
# GMT progs:    makecpt, gmtconvert, grdimage, grdgradient, grdtrack, pstext, psxy
# GMT progs:    pscoast, pstext, psxyz
# Unix progs:   cat, rm
#
ps=example_33.ps

# Extract a subset of ETOPO1m for the East Pacific Rise
# grdcut etopo1m_grd.nc -R118W/107W/49S/42S -Gspac.nc
makecpt -Crainbow -T-5000/-2000/500 -Z > z.cpt
grdgradient spac.nc -A15 -Ne0.75 -Gspac_int.nc
grdimage spac.nc -Ispac_int.nc -Cz.cpt -JM6i -P -Baf -K -Xc --FORMAT_GEO_MAP=dddF -U"Example 33 in Cookbook" > $ps
# Select two points along the ridge
cat << EOF > ridge.txt
-111.6	-43.0
-113.3	-47.5
EOF
# Plot ridge segment and end points
psxy -Rspac.nc -J -O -K -W2p,blue ridge.txt >> $ps
psxy -R -J -O -K -Sc0.1i -Gblue ridge.txt >> $ps
# Generate cross-profiles 400 km long, spaced 10 km, samped every 2km
# and stack these using the median, write stacked profile
grdtrack ridge.txt -Gspac.nc -C400k/2k/10k -Sm+sstack.txt > table.txt
psxy -R -J -O -K -W0.5p table.txt >> $ps
# Show upper/lower values encountered as an envelope
gmtconvert stack.txt -o0,5 > env.txt
gmtconvert stack.txt -o0,6 -I -T >> env.txt
psxy -R-200/200/-3500/-2000 -JX6i/3i -Bafg1000:"Distance from ridge (km)":/af:"Depth (m)":WSne \
	-O -K -Glightgray env.txt -Y6.5i >> $ps
psxy -R -J -O -K -W3p stack.txt >> $ps
echo "0 -2000 MEDIAN STACKED PROFILE" | pstext -R -J -O -K -Gwhite -F+jTC+f14p -Dj0.1i >> $ps
psxy -R -J -O -T >> $ps
# cleanup
rm -f gmt.conf z.cpt spac_int.nc ridge.txt table.txt env.txt stack.txt
