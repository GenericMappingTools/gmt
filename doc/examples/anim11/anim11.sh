#!/usr/bin/env bash
#
# Blending the NASA day and night views from the Blue and Black Marble mosaic
# images using a day-night mask set for the summer solstice midnight in Hawaii
# on June 20, 2020.  In addition, we adjust the colors using the intensities derived
# from the slopes of the earth relief grid.  We spin around at 24 frames per second
# where each frame advances the viewpoint 0.25 degrees of longitude.
#
# DEM:    @earth_relief_02m
# Images: @earth_day_02m @earth_night_02m from the GMT data server
# 
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/nmxy9yb2cR8
# The movie took ~2.75 hour to render on a 24-core MacPro 2013.

# 1. Create background plot and data files needed in the loop
cat << 'EOF' > pre.sh
gmt begin
  # Set view longitudes 0-360 with steps of 0.25 degree
  gmt math -T-180/180/0.25 -o0 T = view.txt
  # Make global grid with a smooth 2-degree day/night transition for the 2020 solstice.
  gmt grdmath -Rd -I2m -rp $(gmt solar -C -o0:1 -I+d2020-06-20+z-10) 2 DAYNIGHT = daynight.grd
  # We will create an intensity grid based on a DEM so that we can see structures in the oceans
  gmt grdgradient @earth_relief_02m -Nt0.75 -A45 -Gintens.grd
  # Make sure our remote files have been downloaded
  gmt which -Ga @earth_day_02m @earth_night_02m
gmt end
EOF
# 2. Set up main script
cat << 'EOF' > main.sh
gmt begin
  # Let HSV minimum value go to zero and faint map border
  gmt set COLOR_HSV_MIN_V 0 MAP_FRAME_PEN=faint
  # Blend the day and night Earth images using the weights, so that when w is 1
  # we get the daytime view, and then adjust colors based on the intensity.
  gmt grdmix @earth_day_02m @earth_night_02m -Wdaynight.grd -Iintens.grd -Gview.tif
  # Plot this image on an Earth with view from current longitude
  gmt grdimage view.tif -JG${MOVIE_COL0}/30N/21.6c -Bafg -X0 -Y0
gmt end
EOF
# 3. Run the movie, requesting a fade in/out via white
gmt movie main.sh -Sbpre.sh -C21.6cx21.6cx50 -Tview.txt -Nanim11 -Lf -Ls"Midnight in Hawaii"+jBR -H8 -Pb+w1c -Fmp4 -V -W -Zs
