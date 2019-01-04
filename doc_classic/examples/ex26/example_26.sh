#!/usr/bin/env bash
#		GMT EXAMPLE 26
#
# Purpose:	Demonstrate general vertical perspective projection
# GMT modules:	pscoast
# Unix progs:	rm
#
ps=example_26.ps

# first do an overhead of the east coast from 160 km altitude point straight down

latitude=41.5
longitude=-74.0
altitude=160.0
tilt=0
azimuth=0
twist=0
Width=0.0
Height=0.0

PROJ=-JG${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/4i

gmt pscoast -Rg $PROJ -X1i -B5g5 -Glightbrown -Slightblue -W -Dl -N1/1p,red -N2,0.5p -P -K \
	-Y5i > $ps

# now point from an altitude of 160 km with a specific tilt and azimuth and with a wider restricted
# view and a boresight twist of 45 degrees

tilt=55
azimuth=210
twist=45
Width=30.0
Height=30.0

PROJ=-JG${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/5i

gmt pscoast -R $PROJ -B5g5 -Glightbrown -Slightblue -W -Ia/blue -Di -Na -O -X1i -Y-4i >> $ps
