#!/usr/bin/env bash
#               GMT ANIMATION 07
#
# Purpose:      Make a Plate Tectonics movie of crustal ages
# GMT modules:  math, set, grdgradient, grdmath, grdimage, makecpt, movie, coast
# Unix progs:   cat
# Note:         Run with any argument to build movie; otherwise one frame is plotted only.

if [ $# -eq 0 ]; then   # Just make master PostScript frame 10
	opt="-M10,ps -Fnone"
	ps=anim07.ps
else	# Make movie in MP4 format and a thumbnail animated GIF using every 10th frame
	opt="-Fmp4 -A+l+s5"
fi
# 1. Create background plot and data files needed in the loop
cat << EOF > pre.sh
gmt begin
	# Set view and sun longitudes
	gmt math -T0/360/5 -I T 5 SUB = longitudes.txt
	# Extract a topography CPT
	gmt makecpt -Cdem2 -T0/6000 -H > t.cpt
	# Get gradients of the relief from N45E
	gmt grdgradient @earth_relief_20m -Nt1.25 -A45 -Gintens.nc
gmt end
EOF
# 2. Set up main script
cat << EOF > main.sh
gmt begin
	# Let HSV minimum value go to zero
	gmt set COLOR_HSV_MIN_V 0
	# Fake simulation of sun illumination from east combined with relief slopes
	gmt grdmath intens.nc X \${MOVIE_COL1} SUB DUP -180 LE 360 MUL ADD 90 DIV ERF ADD 0.25 SUB = s.nc
	# Plot age grid first using age cpt
	gmt grdimage @age.3.20.nc -Is.nc -C@crustal_age.cpt -JG\${MOVIE_COL0}/0/6i -X0 -Y0
	# Clip to expose land areas only
	gmt coast -G
	# Overlay relief over land only using dem cpt
	gmt grdimage @earth_relief_20m -Is.nc -Ct.cpt
	# Undo clipping and overlay gridlines
	gmt coast -Q -B30g30
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Sbpre.sh -C6ix6ix100 -Tlongitudes.txt -Nanim07 -H2 -Pa -Z $opt
rm -rf main.sh pre.sh
