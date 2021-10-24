#!/usr/bin/env bash
#               GMT ANIMATION 04
#
# Purpose:      Make custom full HD movie of NY to Miami night flight
# GMT modules:  grdimage, project, movie
# Unix progs:   cat
#
# Images:		@earth_night from the GMT data server (evaluates to 30s resolution here)
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/j75MbWb4WCE
# The movie took ~9 minute to render on a 24-core MacPro 2013.

# 1. Create files needed in the loop
cat << 'EOF' > pre.sh
# Set up flight path
gmt begin
	gmt project -C-73.8333/40.75 -E-80.133/25.75 -G1 -Q > flight_path.txt
gmt end
EOF
# 2. Set up the main frame script
cat << 'EOF' > main.sh
gmt begin
	gmt grdimage -JG${MOVIE_COL0}/${MOVIE_COL1}/${MOVIE_WIDTH}+du+z160+a210+t55+v36 \
	-Rg @earth_night -Xc -Y-5c
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Chd -Nanim04 -Tflight_path.txt -Sbpre.sh -Zs -H2 -Lf+o0.25c+f14p,Helvetica-Bold,white -Gmidnightblue -V -Fmp4
