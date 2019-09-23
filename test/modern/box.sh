#!/usr/bin/env bash
gmt begin box ps
  gmt subplot begin 2x2 -M0.05i -Fs3i/3i -SCb -SR+tc -R0/5/0/5 -Bwest -T"VERY LONG HEADER STRING"
    gmt basemap -B+t"title 1"
    gmt basemap -c -B+t"title 2"
    gmt basemap -c -Bafg
    gmt basemap -c
  gmt subplot end
gmt end show
