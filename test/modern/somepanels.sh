#!/usr/bin/env bash
# Test only giving a few subplots, Roman labeling, and -A override for one subplot
gmt begin somepanels ps
  gmt subplot begin 3x3 -F6i/8.5i -M5p -A1+c+r+o0.2i+jBR -SCb+lXLABEL+tc -SRl+lYLABEL -Bwstr -T"FIGURE HEADER"
    gmt psbasemap -R0/100/0/10 -c1,1
    gmt subplot 2,2 -Apink
    gmt psbasemap -B+gpink
    gmt psbasemap -c3,3
    gmt psbasemap -R300/500/-10/0 -c2,1
    gmt psbasemap -Bafg -R300/500/-100/0 -c2,3
  gmt subplot end
gmt end
