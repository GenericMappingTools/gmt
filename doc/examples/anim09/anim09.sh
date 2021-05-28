#!/usr/bin/env bash
#
# Animation of a simulated fly-over of the Pacific basin mid-ocean ridge system.
# It uses a premade flight path that originally was derived from a data file
# of the world's ridges, then filtered and manipulated to give the equidistant
# path to simulate a constant velocity at the given altitude, with synthetic
# banking as we turn to follow the path.  We use the 30 arc second global relief
# grid and overlay a few labels for named features.
# DEM:   @earth_relief_30s
# Path:   MOR_PAC_twist_path.txt
# Labels: MOR_names.txt
# We create a global intensity grid using shading from East and a CPT file; these are
# created by the preflight script.
# We add a 1 second fade in and a 1 second fade out for the animation
# The resulting movie was presented at the Fall 2019 AGU meeting in an eLighting talk:
# P. Wessel, 2019, GMT science animations for the masses, Abstract IN21B-11.
# The finished movie is available in our YouTube channel as well (without fading):
# https://youtu.be/LTxlR5LuJ8g
# The movie took ~6 hours to render on a 24-core MacPro 2013.

cat << 'EOF' > pre.sh
#!/usr/bin/env bash
# Pre-script: Runs once to produce files needed for all frames
gmt begin
	gmt grdgradient @earth_relief_30s -A90 -Nt2.5 -Gearth_relief_30s+2.5_int.nc
	gmt makecpt -Cgeo -H > MOR_topo.cpt
gmt end
EOF
cat << 'EOF' > include.sh
# A set of parameters needed by all frames
ALTITUDE=1000
TILT=55
WIDTH=36
HEIGHT=34
EOF
cat << 'EOF' > main.sh
#!/usr/bin/env bash
# Main frame script that makes a single frame given the location and twist from the data file
# and the other view parameters from the include file.
gmt begin
	gmt grdimage -Rg -JG${MOVIE_COL0}/${MOVIE_COL1}/${ALTITUDE}/${MOVIE_COL2}/${TILT}/${MOVIE_COL3}/${WIDTH}/${HEIGHT}/${MOVIE_WIDTH} \
	  -Y0 -X0 @earth_relief_30s -I/tmp/earth_relief_30s+2.5_int.nc -CMOR_topo.cpt
	gmt events MOR_names.txt -L100 -Et+r6+f6 -T${MOVIE_FRAME} -F+f12p,Helvetica-Bold,yellow
gmt end
EOF
# 3. Run the movie
gmt set PROJ_LENGTH_UNIT inch FONT_TAG 20p,Helvetica,white
gmt movie main.sh -Iinclude.sh -CHD -Sbpre.sh -TMOR_PAC_twist_path.txt -Nanim09 -D24 -H4 -Fnone -K -M2000,png -Gblack -Le+jTR -Lf -V -W -Zs
rm -f gmt.conf
