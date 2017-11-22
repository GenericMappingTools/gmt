#!/bin/bash
gmt begin box ps
  gmt subplot begin 2x2 -M0.05i -Fs3i/3i -SCb -SR+tc -R0/5/0/5 -Bwest -T"VERY LONG HEADER STRING"
  gmt basemap -P -c1,1 -B+t"title 1"
  gmt basemap -c1,2 -B+t"title 2"
  gmt basemap -c2,1 -Bafg
  gmt basemap -c2,2
  gmt subplot end
gmt end
