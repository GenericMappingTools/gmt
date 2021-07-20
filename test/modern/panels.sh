#!/usr/bin/env bash
# Test very minimal 2x2 basemap matrix
gmt begin panels ps
  gmt subplot begin 2x2 -Fs3i -M5p -A -Scb -Srl -Bwstr -R0/80/0/10
    gmt basemap
    gmt basemap -c
    gmt basemap -c
    gmt basemap -c
  gmt subplot end
gmt end show
