#!/usr/bin/env bash
#               GMT EXAMPLE 44
#
# Purpose:      Illustrate use of map insets
# GMT modules:  coast, basemap, mapproject
#

gmt begin ex44
	gmt subplot begin 2x1 -Fs15c/10c -B -BWSne
		gmt coast -R10W/5E/35N/44N -JM15c -EES+gbisque -Gbrown -Wfaint -N1/1p -Sazure1 -Df --FORMAT_GEO_MAP=dddF -c0
		gmt inset begin -DjTR+w5c/2.35c+o0.4c/0.25c -F+gwhite+p1p+c0.1c+s
			gmt coast -R15W/35E/30N/48N -JM? -Da -Gbrown -B0 -EES+gbisque --MAP_FRAME_TYPE=plain
		gmt inset end

		gmt coast -R110E/170E/44S/9S -JM15c -Wfaint -N2/1p -EAU+gbisque -Gbrown -Sazure1 -Da --FORMAT_GEO_MAP=dddF -c1
		gmt inset begin -DjTR+w3.8c+o0.4c/0.25c -F+gwhite+p1p+c0.1c+s
			gmt coast -Rg -JG120/30S/? -Da -Gbrown -A5000 -Bg -Wfaint -EAU+gbisque
		gmt inset end
	gmt subplot end
gmt end show
