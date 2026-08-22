#!/usr/bin/env bash
#
# Blending the NASA day and night views from the Blue and Black Marble mosaic
# images using a day-night mask that goes through a full 24-hour cycle.  In addition,
# we adjust the colors using the intensities derived from the slopes of the earth
# relief grid.  We spin around at 24 frames per second where each frame advances by 1 minute.
# Because we are not plotting anything, just manipulating images via grids, we must instead
# create the frames and the final movie via the batch module.
#
# DEM:    @earth_relief_04m
# Images: @earth_day_HD.tif @earth_night_HD.tif from the GMT cache server
# 
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/X8TojLs0NYk
# The movie took ~4 minutes to render on a 24-core MacPro 2013.
# To make a UHD (4k) movie, comment/uncomment the SIZE, INC and FILT settings
cat << 'EOF' > inc.sh
SIZE=HD
INC=11.25
FILT=21
# SIZE=UHD
# INC=5.625
# FILT=10.5
EOF
# 1. Create preflight script to build data files needed in the loop
cat << 'EOF' > pre.sh
gmt begin
  # Set view time for June 20-21, 2020 with steps of 1 minute
  gmt math -T2020-06-20T/2020-06-21T/1 -o0 --TIME_UNIT=m  T = times.txt
  # Create a ${SIZE} DEM at ${INC}x${INC} arc minutes to yield ${SIZE} dimensions
  gmt grdfilter @earth_relief_04m -Fg${FILT} -I${INC}m -D1 -r -Gtopo_${SIZE}.grd
  # Create an intensity grid based on the ${SIZE} DEM so we can see structures in the oceans
  gmt grdgradient topo_${SIZE}.grd -Nt0.5 -A45 -Gintens_${SIZE}.grd
  # Make sure our remote files have been downloaded
  gmt which -Ga @earth_day_${SIZE}.tif @earth_night_${SIZE}.tif
gmt end
EOF
# 2. Set up main script
cat << 'EOF' > main.sh
gmt begin
  # Let HSV minimum value go to zero and faint map border
  gmt set COLOR_HSV_MIN_V 0 MAP_FRAME_PEN=faint
  # Make global grid with a smooth 2-degree day/night transition for this time.
  gmt grdmath -Rd -I${INC}m -r $(gmt solar -C -o0:1 -I+d${BATCH_COL0}) 2 DAYNIGHT = daynight_${SIZE}.grd
  # Blend the day and night Earth images using the weights, so that when w is 1
  # we get the daytime view, and then adjust colors based on the intensity.
  gmt grdmix @earth_day_${SIZE}.tif @earth_night_${SIZE}.tif -Wdaynight_${SIZE}.grd -Iintens_${SIZE}.grd -G${BATCH_NAME}.png -Ve
gmt end
EOF
# 3. Create postflight script to build movie from the images
cat << 'EOF' > post.sh
gmt begin
  prec=$(gmt math -Q ${BATCH_NJOBS} LOG10 CEIL =)
  ffmpeg -loglevel warning  -f image2 -framerate 24 -y -i "${BATCH_PREFIX}_%0${prec}d.png" -vcodec libx264 -pix_fmt yuv420p ${BATCH_PREFIX}_${SIZE}.mp4
  rm -f ${BATCH_PREFIX}_*.png
gmt end
EOF
# 4. Run the batch, requesting a fade in/out via white
gmt batch main.sh -Iinc.sh -Sbpre.sh -Sfpost.sh -Ttimes.txt -Nanim12 -V -W -Zs
