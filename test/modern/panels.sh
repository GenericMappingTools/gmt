#!/usr/bin/env bash
# Test very minimal 2x2 basemap matrix
gmt begin panels ps
  gmt subplot begin 2x2 -Fs3i -M5p -A -SCb -SRl -Bwstr
    gmt psbasemap -R0/80/0/10 -c
    gmt psbasemap -c
    gmt psbasemap -c
    gmt psbasemap -c
  gmt subplot end
gmt end
