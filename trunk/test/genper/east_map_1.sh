#!/bin/bash
#	$Id: east_map_1.sh,v 1.5 2011-06-23 17:52:02 remko Exp $

. ../functions.sh
header "Test -JG (US East Coast 160 km)"

EARTH_MODEL=e
DEBUG=
X0=-Xc
Y0=-Yc
REGION=-Rg
PSFILE=east_map_1
TITLE=:.${PSFILE}:
latitude=41.5
longitude=-74.0
altitude=160.0
tilt=0
azimuth=0
twist=0
Width=0.0
Height=0.0

# now point from an altitude of 160 km 

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

pscoast ${GMT_VERBOSE} $REGION $PROJ -P -Yc -Xc -B5g5/5g5${TITLE} -G128/255/128 -S128/128/255 -W -Ia -Di -Na --MAP_ANNOT_MIN_SPACING=0.5i > $PSFILE.ps

pscmp $PSFILE
