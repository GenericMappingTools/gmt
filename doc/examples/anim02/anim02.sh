#!/usr/bin/env bash
#               GMT ANIMATION 02
#
# Purpose:      Make simple MP4 of an illuminated DEM grid of the US Rockies
# GMT modules   math, makecpt, grdcut, grdimage, inset, plot, text, movie
# Unix progs:   cat
#
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/WQ6JrtEu_Fk
# The movie took 1 minute 45 seconds to render on a 24-core MacPro 2013.

# 1. Create files needed in the loop
cat << 'EOF' > pre.sh
gmt begin
	gmt math -T0/360/1 T -o0 = angles.txt
	gmt makecpt -Coleron -T500/4500 -H > main.cpt
	gmt grdcut @earth_relief_15s -R-114/-103/35/40 -Gtopo.nc
gmt end
EOF
# 2. Set up the main frame script
cat << 'EOF' > main.sh
gmt begin
	width=$(gmt math -Q ${MOVIE_WIDTH} 1.5c SUB =)
	gmt grdimage topo.nc -I+a${MOVIE_COL0}+nt2 -JM${width} -Cmain.cpt \
		-BWSne -B1 -X1c -Y0.53c --FONT_ANNOT_PRIMARY=9p
	gmt inset begin -DjBR+w2.1c/2.4c+o0.2c -F+gwhite+s+p
	echo 0.5 0.5 | gmt plot -R0/1/0/1 -JX? -Sc2c -Gwhite -Wthinner -Y-0.15
		echo 0.5 0.5 ${MOVIE_COL0} | gmt plot -Sk@azimuth/1.0c -W1.75p -Gred
		echo Light Source | gmt text -F+cTC+f9 -Y0.1
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Chd -Nanim02 -Tangles.txt -Vi -Sbpre.sh -Zs -Fmp4
