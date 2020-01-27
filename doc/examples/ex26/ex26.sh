#!/usr/bin/env bash
#		GMT EXAMPLE 26
#
# Purpose:	Demonstrate general vertical perspective projection
# GMT modules:	coast
#
gmt begin ex26
	# first do an overhead of the east coast from 160 km altitude point straight down
	latitude=41.5
	longitude=-74
	altitude=160
	tilt=0
	azimuth=0
	twist=0
	Width=0
	Height=0

	PROJ=-JG${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/10c
	gmt coast -Rg $PROJ -B5g5 -Glightbrown -Slightblue -W -Dl -N1/1p,red -N2/0.5p -Y12c

	# now point from an altitude of 160 km with a specific tilt and azimuth and with a wider restricted
	# view and a boresight twist of 45 degrees
	tilt=55
	azimuth=210
	twist=45
	Width=30
	Height=30

	PROJ=-JG${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/12c
	gmt coast $PROJ -B5g5 -Glightbrown -Slightblue -W -Ia/blue -Di -Na -X1i -Y-10c
gmt end show
