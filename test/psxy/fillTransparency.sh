#!/usr/bin/env -S bash -ex
# GMT modern mode bash template
# Date:    2026-01-21T12:44:22
# User:    solarsmith
# Purpose: Purpose of this script
set -e
export GMT_SESSION_NAME=$$	# Set a unique session name

gmt begin fillTransparency ps
  gmt basemap -R-2/2/-2/2 -JX10c/4c -B
  echo -1 0 | gmt plot -Ss1c -W1p,blue@60 -Gbrown
  echo 0 0 | gmt plot -Ss1c -W1p,blue -Gbrown@30
  echo 1 0 | gmt plot -Ss1c -W1p,blue@60 -Gbrown@30
gmt end