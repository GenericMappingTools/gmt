#!/usr/bin/env bash
# Test very minimal 2x2 basemap matrix with log10 and power axes
gmt begin powlog ps
  gmt subplot begin 2x2 -Fs3i -M5p -A -SCb -SRl -Bwstr
    gmt basemap -R1/100/0/100 -JX?l/?p0.5
    gmt basemap -c
    gmt basemap -R0/100/1/100 -JX?p0.5/?l -c
    gmt basemap -c
  gmt subplot end
gmt end show
