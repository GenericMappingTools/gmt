#!/usr/bin/env bash
#               GMT ANIMATION 04
#
# Purpose:      Make custom 480p movie of NY to Miami flight
# GMT modules:  set, grdgradient, grdimage, makecpt, project, plot, movie
# Unix progs:   cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted as PS only.

if [ $# -eq 0 ]; then	# Just make master PostScript frame 0
	opt="-Mps -Fnone"
	ps=anim04.ps
else	# Make movie in MP4 format and a thumbnail animated GIF using every 10th frame
	opt="-Fmp4 -A+l+s10"
fi
# 1. Create files needed in the loop
cat << EOF > pre.sh
# Set up flight path
gmt begin
	gmt project -C-73.8333/40.75 -E-80.133/25.75 -G10 -Q > flight_path.txt
	gmt grdgradient @USEast_Coast.nc -A90 -Nt1 -Gint_US.nc
	gmt makecpt -Cglobe -H > globe_US.cpt
gmt end
EOF
# 2. Set up the main frame script
cat << EOF > main.sh
gmt begin
	gmt grdimage -JG\${MOVIE_COL0}/\${MOVIE_COL1}/160/210/55/0/36/34/\${MOVIE_WIDTH}+ \
		-Rg @USEast_Coast.nc -Iint_US.nc -Cglobe_US.cpt -X0 -Y0
	gmt plot -W1p flight_path.txt
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -C7.2ix4.8ix100 -Nanim04 -Tflight_path.txt -Sbpre.sh -Z -H2 -Lf+o0.1i+f14p,Helvetica-Bold $opt
rm -f main.sh pre.sh
