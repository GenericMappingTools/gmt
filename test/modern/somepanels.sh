#!/usr/bin/env bash
# Test only giving a few subplots, Roman labeling, and -A override for one subplot
gmt begin somepanels ps
  gmt subplot begin 3x3 -F6i/8.5i -M5p -A1+c+r+o0.2i+jBR -SCb+lXLABEL -SRl+lYLABEL -Bwstr -T"FIGURE HEADER"
    gmt basemap -R0/100/0/10 -c0
    gmt subplot 1,1 -Apink
    gmt basemap -B+gpink
    gmt basemap -c2,2
    gmt basemap -R300/500/-10/0 -c1,0
    gmt basemap -Bafg -R300/500/-100/0 -c5
  gmt subplot end
gmt end show
