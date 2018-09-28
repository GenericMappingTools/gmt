#!/bin/bash
gmt begin GMT_panel ps
gmt basemap -R0/2/0/1 -JX5i/2i -B0 -DjTL+o0.2i+w1.75i/0.75i -F+glightgreen+r 
gmt basemap -DjBR+o0.3i+w1.75i/0.75i -F+p1p+i+s+gwhite+c0.1i 
gmt basemap -DjBR+o0.3i+w1.75i/0.75i -F+p0.25p,-+c0 
gmt end
