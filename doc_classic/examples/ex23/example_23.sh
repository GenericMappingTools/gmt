#!/usr/bin/env bash
#		GMT EXAMPLE 23
#
# Purpose:	Plot distances from Rome and draw shortest paths
# GMT modules:	grdmath, grdcontour, pscoast, psxy, pstext, grdtrack
# Unix progs:	echo, cat
#
ps=example_23.ps

# Position and name of central point:

lon=12.50
lat=41.99
name="Rome"

# Calculate distances (km) to all points on a global 1x1 grid

gmt grdmath -Rg -I1 $lon $lat SDIST = dist.nc

# Location info for 5 other cities + label justification

cat << END > cities.txt
105.87	21.02	LM	HANOI
282.95	-12.1	LM	LIMA
178.42	-18.13	LM	SUVA
237.67	47.58	RM	SEATTLE
28.20	-25.75	LM	PRETORIA
END

gmt pscoast -Rg -JH90/9i -Glightgreen -Sblue -A1000 -Dc -Bg30 \
	-B+t"Distances from $name to the World" -K -Wthinnest > $ps

gmt grdcontour dist.nc -A1000+v+u" km"+fwhite -Glz-/z+ -S8 -C500 -O -K -J \
	-Wathin,white -Wcthinnest,white,- >> $ps

# For each of the cities, plot great circle arc to Rome with gmt psxy
gmt psxy -R -J -O -K -Wthickest,red -Fr$lon/$lat cities.txt >> $ps

# Plot red squares at cities and plot names:
gmt psxy -R -J -O -K -Ss0.2 -Gred -Wthinnest cities.txt >> $ps
gmt pstext -R -J -O -K -Dj0.15/0 -F+f12p,Courier-Bold,red+j -N cities.txt >> $ps
# Place a yellow star at Rome
echo "$lon $lat" | gmt psxy -R -J -O -K -Sa0.2i -Gyellow -Wthin >> $ps

# Sample the distance grid at the cities and use the distance in integer km for labels

gmt grdtrack -Gdist.nc cities.txt -o0-2 --FORMAT_FLOAT_OUT=0:%g,1:%g,2:%.0f \
	| gmt pstext -R -J -O -D0/-0.2i -N -Gwhite -W -C0.02i -F+f12p,Helvetica-Bold+jCT >> $ps

# Clean up after ourselves:

rm -f cities.txt dist.nc
