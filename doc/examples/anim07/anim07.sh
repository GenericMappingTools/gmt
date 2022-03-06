#!/usr/bin/env bash
#
## Animation of a spinning Earth showing the crustal ages from EarthByte for the oceans
# and topographic relief on land, with shading given by the global relief and modified
# by position relative to an artificial sun in the east.  A progress slice is added as well.
# We add a 1 second fade in and a 1 second fade out for the animation
# DEM:   @earth_relief_06m
# Ages:  @earth_age_06m
# A similar movie was presented at the Fall 2019 AGU meeting in an eLighting talk:
# P. Wessel, 2019, GMT science animations for the masses, Abstract IN21B-11.
# The finished movie is available in our YouTube channel as well (without fading):
# https://youtu.be/KfBwQlyjz5w
# The movie takes about ~20 minutes to render on a 24-core MacPro 2013.
# The higher resolution movie on YouTube used the 02m data set resolution.

# 1. Create background plot and data files needed in the loop
cat << 'EOF' > pre.sh
gmt begin
	# Set view and sun longitudes
	gmt math -T-12/372/0.5 -I T 5 SUB = longitudes.txt
	# Extract a topography CPT
	gmt makecpt -Cdem2 -T0/6000 -H > z.cpt
	# Get gradients of the relief from N45E
	gmt grdgradient @earth_relief_06m -Nt1.2 -A45 -Gintens.grd
gmt end
EOF
# 2. Set up main script
cat << 'EOF' > main.sh
gmt begin
	# Let HSV minimum value go to zero and faint map border
	gmt set COLOR_HSV_MIN_V 0 MAP_FRAME_PEN=faint
	# Fake simulation of sun illumination from east added to relief intensities
	gmt grdmath intens.grd X ${MOVIE_COL1} SUB SIND 0.8 MUL ADD 0.2 SUB = s.nc
	# Plot age grid first using EarthByte age cpt
	gmt grdimage @earth_age_06m -Is.nc -JG${MOVIE_COL0}/15/10.8c -X0 -Y0
	# Clip to expose land areas only
	gmt coast -G -Di
	# Overlay relief over land only using dem cpt
	gmt grdimage @earth_relief_06m -Is.nc -Cz.cpt
	# Undo clipping and overlay gridlines
	gmt coast -Q -B30g30
gmt end show
EOF
# 3. Run the movie, requesting a fade in/out via white
gmt movie main.sh -Sbpre.sh -C10.8cx10.8cx100 -Tlongitudes.txt -Nanim07 -Lf -K+gwhite -H8 -Pa+w1c+Gwhite -Fmp4 -V -W -Zs
