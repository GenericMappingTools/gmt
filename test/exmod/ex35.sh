#!/usr/bin/env bash
#               GMT EXAMPLE 35
#
# Purpose:      Illustrate sphtriangulate and sphdistance with GSHHG crude data
# GMT modules:  coast, plot, makecpt, grdimage, grdcontour, sphtriangulate, sphdistance
# Unix progs:   rm
#
gmt begin ex35 ps

	# Get the crude GSHHS data, select GMT format, and decimate to ~20%:
	# gshhs $GMTHOME/src/coast/gshhs/gshhs_c.b | $AWK '{if ($1 == ">" || NR%5 == 0) print $0}' > gshhs_c.txt
	# Get Voronoi polygons
	gmt sphtriangulate @gshhs_c.txt -Qv -D > tt.pol
	# Compute distances in km
	gmt sphdistance -Rg -I1 -Qtt.pol -Gtt.nc -Lk
	gmt makecpt -Chot -T0/3500
	# Make a basic image plot and overlay contours, Voronoi polygons and coastlines
	gmt grdimage tt.nc -JG-140/30/7i -X0.75i -Y2i
	gmt grdcontour tt.nc -C500 -A1000+f10p,Helvetica,white -L500 -GL0/90/203/-10,175/60/170/-30,-50/30/220/-5 -Wa0.75p,white -Wc0.25p,white
	gmt plot tt.pol -W0.25p,green,.
	gmt coast -W1p -Gsteelblue -A0/1/1 -B30g30 -B+t"Distances from GSHHG crude coastlines"
	# cleanup
	rm -f tt.pol tt.nc
gmt end
