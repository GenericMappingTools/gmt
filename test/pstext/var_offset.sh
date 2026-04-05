#!/usr/bin/env bash
# Test pstext -D+f<file> for per-record variable offsets
# Each text label gets its own (dx,dy) displacement from an offset file
gmt begin var_offset
	# Create the offset file: 4 records with different (dx,dy) values (in cm)
	gmt set PROJ_LENGTH_UNIT=cm
	cat > offsets.txt <<- EOF
		0.3   0.3
		-0.3  0.3
		-0.3 -0.3
		0.3  -0.3
	EOF
	# Plot base map and mark the anchor points
	gmt basemap -R-5/5/-5/5 -JX10c -Bafg5 -BWSne+t"Variable offsets (-D+f)"
	# Draw small circles at the 4 anchor positions
	gmt plot -Sc0.15c -Gred -Wfaint <<- EOF
		-3	-3
		-3	3
		3	3
		3	-3
	EOF
	# Draw connecting lines from anchor to offset position to visualise displacement
	# (not strictly needed but shows the offset)
	# Place text labels using per-record variable offsets from offsets.txt
	gmt text -F+f12p,Helvetica-Bold,blue+jCM -D+v+foffsets.txt <<- EOF
		-3	-3	SW corner
		-3	3	NW corner
		3	3	NE corner
		3	-3	SE corner
	EOF
	rm -f offsets.txt
gmt end show
