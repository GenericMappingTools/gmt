#!/bin/bash
#               GMT ANIMATION 07
#               $Id$
#
# Purpose:      Make a Plate Tectonics movie of crustal ages
# GMT modules:  gmtmath, gmtset, grdgradient, grdmath, grdimage, makecpt, movie, pscoast
# Unix progs:   cat
# Note:         Run with any argument to build movie; otherwise one frame is plotted only.

if [ $# -eq 1 ]; then   # Just make master PostScript frame 30
        opt="-M30,ps -Fnone"
else    # Make both movie formats and a thumbnail animated GIF using every 20th frame
        opt="-Fmp4 -A+l+s10"
fi
# 1. Create background plot and data files needed in the loop
cat << EOF > pre.sh
# Set view and sun longitudes
gmt math -T0/360/1 -I T 5 SUB = longitudes.txt
# Exctact a topography CPT
gmt makecpt -Cdem2 -T0/6000 > movie_dem.cpt
# Get gradients of the relief from N45E
gmt grdgradient @earth_relief_10m -Nt1.25 -A45 -Gintens.nc
EOF
# 2. Set up main script
cat << EOF > main.sh
gmt begin
	# Let HSV minimum value go to zero
	gmt set COLOR_HSV_MIN_V 0
	# Fake simulation of sun illumination from east combined with relief slopes
	gmt grdmath intens.nc X \${GMT_MOVIE_VAL2} SUB DUP -180 LE 360 MUL ADD 90 DIV ERF ADD 0.25 SUB = s.nc
	# Plot age grid first using age cpt
	gmt grdimage @age.3.10.nc -Is.nc -C@crustal_age.cpt -JG\${GMT_MOVIE_VAL1}/0/6i -X0 -Y0
	# Clip to expose land areas only
	gmt pscoast -Gc
	# Overlay relief over land only using dem cpt
	gmt grdimage @earth_relief_10m -Is.nc -Cmovie_dem.cpt
	# Undo clipping and overlay gridlines
	gmt pscoast -Q -B30g30
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Sbpre.sh -C6ix6ix100 -Tlongitudes.txt -Nanim_07 -D24 $opt -W4
rm -rf main.sh pre.sh
