#!/bin/bash
# Main frame script that makes a single frame given the location and twist from the data file
# and the other view parameters from the include file.
gmt begin
	gmt grdimage -Rg -JG${MOVIE_COL0}/${MOVIE_COL1}/${ALTITUDE}/${MOVIE_COL2}/${TILT}/${MOVIE_COL3}/${WIDTH}/${HEIGHT}/${MOVIE_WIDTH} 	  -Y0 -X0 @earth_relief_30s.grd -I/tmp/earth_relief_30s+2.5_int.nc -CMOR_topo.cpt
	gmt events MOR_names.txt -L100 -Et+r6+f6 -T${MOVIE_FRAME} -F+f12p,Helvetica-Bold,yellow
gmt end
