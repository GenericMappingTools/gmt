#!/bin/bash
# Test very minimal 2x2 basemap matrix
gmt begin panels ps
  gmt subplot begin 2x2 -Fs3i -M5p -A -SCb -SRl -Bwstr
    gmt psbasemap -R0/80/0/10 -c1,1
    gmt psbasemap -c1,2
    gmt psbasemap -c2,1
    gmt psbasemap -c2,2
  gmt subplot end
gmt end
