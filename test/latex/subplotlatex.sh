#!/usr/bin/env bash
# Test of user Latex labels in subplot settings
gmt begin subplotlatex ps
  gmt subplot begin 2x2 -Fs8c -M5p -SCb -SRl -BeSWn -JX10c -R-30/30/0/80 -Bxafg+l"@[\int_{x} \int_{t} \tau_x@[" -Byafg+l"<math>\nabla G</math>"
    gmt basemap -c
    gmt basemap -c
    gmt basemap -c
    gmt basemap -c
  gmt subplot end
gmt end show
