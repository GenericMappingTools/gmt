#!/usr/bin/env bash
# Test various -J perturbations
gmt begin linpanelsnew ps
  gmt set MAP_FRAME_TYPE plain
  gmt subplot begin 3x3 -Fs5c/7c -A1 -M6p -R0/100/0/80 -BWSen
    gmt basemap -c
    gmt basemap -c -JX?
    gmt basemap -c -JX?/?
    
    gmt basemap -c -JX-?/?
    gmt basemap -c -JX?/-?
    gmt basemap -c -JX-?/-?
    
    gmt basemap -c -JX?d/?
    gmt basemap -c -JX?/?d
    gmt basemap -c -JX?d/?d
  gmt subplot end
gmt end show
