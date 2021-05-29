#!/usr/bin/env bash
# Testing gmt subplot 12 cm circles and framepen scaling

gmt begin circles ps
  gmt subplot begin 2x1 -Fs12c -Baf -Rg -M0 -Y1.25c
    gmt basemap -JG0/10/? -c
    gmt basemap -JG0/-10/? -c
  gmt subplot end
gmt end show
