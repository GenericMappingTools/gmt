#!/usr/bin/env bash
#               GMT ANIMATION 03
#
# Purpose:      Make simple animated MP4 of Iceland topo from different viewpoints
# GMT modules:  basemap, grdclip, grdcut, grdgradient, makecpt, math, grdview, movie
# Unix progs:   echo, cat
# Note:         Run with any argument to build movie; otherwise 27th frame is plotted only.
#
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/zCo0fuMkvMY
# The movie took just 2 minutes to render on a 24-core MacPro 2013.

if [ $# -eq 0 ]; then	# Just make master PostScript frame 27 (az = 206)
	opt="-M27,ps"
else	# Make MP4
	opt="-Fmp4"
fi
# 1. Create files needed in the loop
cat << 'EOF' > pre.sh
gmt begin
	gmt math -T180/540/1 T 30 SUB = angles.txt
	gmt makecpt -Coleron -T-2000/2000/20 -H > iceland.cpt
	gmt grdcut @earth_relief_15s -R-26/-12/63/67 -Giceland.nc
	gmt grdclip iceland.nc -Gabove.nc -Sb0/NAN -Sr0/NAN
	gmt grdclip iceland.nc -Gbelow.nc -Sa0/NAN
gmt end
EOF
# 2. Set up the main frame script
cat << 'EOF' > main.sh
gmt begin
	gmt grdgradient iceland.nc -Nt1 -Gintens.nc -A-${MOVIE_COL1}
	gmt grdview below.nc -Rbelow.nc -Ciceland.cpt -Qc${MOVIE_DPU} -Iintens.nc -JM19c  -p-${MOVIE_COL0}/35+v4.5c/1.7c -X7.6c -Y5c
	gmt grdview above.nc -Rabove.nc -Ciceland.cpt -Qc${MOVIE_DPU} -Iintens.nc -JZ0.4c -p 
	gmt basemap -Bf -Tdn0.06/0.8+w1.75c+f+l,,,N -p
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Chd -Nanim03 -Tangles.txt -Sbpre.sh -Vi -Pb -Zs $opt
