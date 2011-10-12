#!/bin/bash
#	$Id$
# Test of projection

. ../functions.sh
header "Test -JG (US East Coast 20000 km)"

EARTH_MODEL=e
DEBUG=
X0=-Xc
Y0=-Yc
REGION=-Rg
PSFILE=east_map_0
TITLE=:.${PSFILE}:
latitude=41.5
longitude=-74.0
altitude=20000.0
tilt=0
azimuth=0
twist=0
Width=0.0
Height=0.0

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

pscoast $REGION $PROJ -P -Yc -Xc -B10g10/10g10${TITLE} -G128/255/128 -S128/128/255 -W -Ia -Di -Na --MAP_ANNOT_MIN_SPACING=0.5i > $PSFILE.ps

pscmp $PSFILE
