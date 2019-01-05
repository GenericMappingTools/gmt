#!/usr/bin/env bash
#               GMT ANIMATION 04
#
# Purpose:      Make custom 480p movie of NY to Miami flight
# GMT modules:  gmtset, grdgradient, grdimage, makecpt, project, psxy, movie
# Unix progs:   cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted as PS only.

if [ $# -eq 0 ]; then	# Just make master PostScript frame 0
	opt="-Mps -Fnone"
	ps=anim_04.ps
else	# Make both movie formats and a thumbnail animated GIF using every 20th frame
	opt="-Fmp4 -Fwebm -A+l+s20"
fi	
# 1. Create files needed in the loop
cat << EOF > pre.sh
# Set up flight path
gmt project -C-73.8333/40.75 -E-80.133/25.75 -G5 -Q > flight_path.txt
gmt grdgradient @USEast_Coast.nc -A90 -Nt1 -Gint_US.nc
gmt makecpt -Cglobe > globe_US.cpt
EOF
# 2. Set up the main frame script
cat << EOF > main.sh
gmt begin
	gmt set FONT_TAG 14p,Helvetica-Bold
	gmt grdimage -JG\${MOVIE_COL1}/\${MOVIE_COL2}/160/210/55/0/36/34/\${MOVIE_WIDTH}+ \
		-Rg @USEast_Coast.nc -Iint_US.nc -Cglobe_US.cpt -X0 -Y0
	gmt psxy -W1p flight_path.txt
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -C7.2ix4.8ix100 -Nanim_04 -Tflight_path.txt -Sbpre.sh -Z -H2 -Lf+o0.1i $opt
rm -f main.sh pre.sh
