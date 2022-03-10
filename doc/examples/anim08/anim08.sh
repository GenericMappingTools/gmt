#!/usr/bin/env bash
#
# Animation of all seismicity in the Pacific basin during 2018 that exceeded
# a threshold of magnitude 5.  We then plot the earthquakes as they occurred on
# a map that moves in longitude from west to east, with a deliberate pause near
# Longitude 200 so we can watch the near-daily magnitude 5.2 quakes that hit the
# Big Island of Hawaii during the 3-month 2018 eruption.  We scale magnitude to
# symbol size (spherical circle diameter in km) and announce each quake by magnifying
# size and whitening the color for a little bit.  Later the symbols fade to darker
# color and smaller sizes but remain for the duration of the movie.
# DEM:   @earth_relief_02m
# Line:  ridge.txt
# The resulting movie was presented at the Fall 2019 AGU meeting in an eLighting talk:
# P. Wessel, 2019, GMT science animations for the masses, Abstract IN21B-11.
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/H0RyjHRhJ3g
# The movie took ~1 hour to render on a 24-core MacPro 2013.

# 1. Extract 2018 data >= Mag 5 from URL and prepare inputs and frame times
cat << 'EOF' > pre.sh
SITE="https://earthquake.usgs.gov/fdsnws/event/1/query.csv"
TIME="starttime=2018-01-01%2000:00:00&endtime=2018-12-31%2000:00:00"
MAG="minmagnitude=5"
ORDER="orderby=time-asc"
URL="${SITE}?${TIME}&${MAG}&${ORDER}"
gmt begin
	# Get seismicity and scale magnitude by 50 to give symbol size in km
	gmt convert $URL -i2,1,3,4+s50,0 -hi1 > q.txt
	# Create standard seismicity cpt
	gmt makecpt -Cred,green,blue -T0,70,300,10000 -H > q.cpt
	# Make lons go from 160 to 240 with a pause at 200 mid movie
	cat <<- EOF | gmt sample1d -fT --TIME_UNIT=d -I1 -Fa > times.txt
	2018-01-01T	160
	2018-05-01T	200
	2018-06-01T	200
	2018-07-01T	200
	2018-08-01T	200
	2018-12-31T	240
	EOF
    # Get gradients of the relief from N45E
    gmt grdgradient @earth_relief_02m -Nt1.2 -A45 -Gintens.nc
	gmt makecpt -Cterra -T-10000/8000 -H > t.cpt
gmt end
EOF
# 2. Set up main script
cat << 'EOF' > main.sh
gmt begin
	# Let HSV minimum value go to zero
	gmt set COLOR_HSV_MIN_V 0
	# Fake simulation of sun illumination from east combined with relief slopes
	gmt grdmath intens.nc X ${MOVIE_COL1} SUB DUP -180 LE 360 MUL ADD 90 DIV ERF ADD 0.5 SUB = s.nc
	# Overlay relief over land only using dem cpt
	gmt grdimage @earth_relief_02m -Is.nc -Ct.cpt -JG${MOVIE_COL1}/5/18c -X0 -Y0
	# Plot the mid-ocean ridge line
	gmt plot @ridge.txt -W0.5p,darkyellow
	# Plot seismicity at this time according to visibility and enhancement settings
	gmt events q.txt -SE- -Cq.cpt --TIME_UNIT=d -T${MOVIE_COL0} -Es+r2+d6 -Ms5+c0.5 -Mi1+c-0.6 -Mt+c0
gmt end
EOF
# 3. Run the movie at 24 frames/sec at 60 dpc to give 1080x1080 size, each frame represent one day
gmt movie main.sh -Sbpre.sh -C18cx18cx60 -Ttimes.txt -Nanim08 -H8 -Lc0 -M150,png -Fmp4 -G30 -V -W -Zs \
	--FONT_TAG=20p,Helvetica,white --FORMAT_CLOCK_MAP=-
