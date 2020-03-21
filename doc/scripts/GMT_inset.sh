#!/usr/bin/env bash
#
# Testing gmt legend capabilities for tables with colors

gmt begin GMT_inset
# Bottom map of Australia
	gmt coast -R110E/170E/44S/9S -JM6i -B -BWSne -Wfaint -N2/1p  -EAU+gbisque -Gbrown -Sazure1 -Da -Xc --FORMAT_GEO_MAP=dddF
	gmt inset begin -DjTR+w1.5i+o0.15i -F+gwhite+p1p+s -M0.05i
	gmt coast -Rg -JG120/30S/1.4i -Da -Gbrown -A5000 -Bg -Wfaint -EAU+gbisque
	gmt inset end
gmt end show
