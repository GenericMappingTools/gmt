#!/usr/bin/env bash
# Test the new +f modifier to -B
gmt begin fancyannot ps
  gmt subplot begin 2x1 -M12p -Fs6i/4i -SR+ -R-1.5/1.5/-1/1 -BWSne -T"FANCY ANNOTATIONS"
    gmt basemap -JM? -B+f 
    gmt basemap -JM? -B -c
  gmt subplot end
gmt end show
