#!/bin/bash
#	$Id$

. ../functions.sh
header "Test -JG (US East Coast 160 km w/tilt+view2)"

EARTH_MODEL=e
DEBUG=
X0=-Xc
Y0=-Yc
REGION=-Rg
PSFILE=east_map_3
TITLE=:.${PSFILE}:
latitude=41.5
longitude=-74.0
altitude=160.0
tilt=55
azimuth=210
twist=0
Width=30.0
Height=30.0

# now point from an altitude of 160 km with a specific tilt and azimuth but with an unrestricted field of view

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

pscoast ${GMT_VERBOSE} $REGION $PROJ -P -Yc -Xc -B5g5/5g5${TITLE} -G128/255/128 -S128/128/255 -W -Ia -Di -Na --MAP_ANNOT_MIN_SPACING=0.5i > $PSFILE.ps

pscmp $PSFILE
