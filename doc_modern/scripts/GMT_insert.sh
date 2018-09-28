#!/bin/bash
#
# Testing gmt legend capabilities for tables with colors

gmt begin GMT_insert ps
# Bottom map of Australia
gmt coast -R110E/170E/44S/9S -JM6i -Baf -BWSne -Wfaint -N2/1p  -EAU+gbisque -Gbrown -Sazure1 -Da -Xc --FORMAT_GEO_MAP=dddF
gmt basemap -DjTR+w1.5i+o0.15i+stmp -F+gwhite+p1p+c0.1c+s
read x0 y0 w h < tmp
gmt coast -Rg -JG120/30S/$w -Da -Gbrown -A5000 -Bg -Wfaint -EAU+gbisque -X$x0 -Y$y0 
gmt plot -T  -X-${x0} -Y-${y0} 
gmt end
