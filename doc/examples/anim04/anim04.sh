#!/usr/bin/env bash
#               GMT ANIMATION 04
#
# Purpose:      Make custom full hd movie of NY to Miami flight
# GMT modules:  set, grdimage, project, plot, movie
# Unix progs:   cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted as PS only.

if [ $# -eq 0 ]; then	# Just make master PostScript frame 0
	opt="-Mps"
else	# Make movie in MP4 format and a thumbnail animated GIF using every 10th frame
	opt="-Fmp4 -Fgif+l+s10"
fi
# 1. Create files needed in the loop
cat << 'EOF' > pre.sh
# Set up flight path
gmt begin
	gmt project -C-73.8333/40.75 -E-80.133/25.75 -G10 -Q > flight_path.txt
gmt end
EOF
# 2. Set up the main frame script
cat << 'EOF' > main.sh
gmt begin
	gmt grdimage -JG${MOVIE_COL0}/${MOVIE_COL1}/${MOVIE_WIDTH}+du+z160+a210+t55+v36 \
	-Rg @earth_night_01m -Xc -Y-5c
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Chd -Nanim04 -Tflight_path.txt -Sbpre.sh -Zs -H2 -Lf+o0.25c+f14p,Helvetica-Bold,white -Gmidnightblue $opt
