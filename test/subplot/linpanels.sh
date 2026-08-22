#!/usr/bin/env bash
# Test various -J perturbations without the ? marks
gmt begin linpanels ps
  gmt set MAP_FRAME_TYPE plain
  gmt subplot begin 3x3 -Fs5c/7c -A1 -M6p -R0/100/0/80 -BWSen
    gmt basemap -c
    gmt basemap -c -JX
    gmt basemap -c -JX/
    
    gmt basemap -c -JX-/
    gmt basemap -c -JX/-
    gmt basemap -c -JX-/-
    
    gmt basemap -c -JXd/
    gmt basemap -c -JX/d
    gmt basemap -c -JXd/d
  gmt subplot end
gmt end show
