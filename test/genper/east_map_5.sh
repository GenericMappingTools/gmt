#!/bin/bash
#	$Id: east_map_5.sh,v 1.5 2011-06-23 17:52:02 remko Exp $

. ../functions.sh
header "Test -JG (US East Coast 160 km w/tilt+twist)"

EARTH_MODEL=e
DEBUG=
X0=-Xc
Y0=-Yc
REGION=-Rg
PSFILE=east_map_5
TITLE=:.${PSFILE}:
latitude=41.5
longitude=-74.0
altitude=160.0
tilt=55
azimuth=210
twist=45
Width=30.0
Height=30.0

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

pscoast ${GMT_VERBOSE} $REGION $PROJ -P -Yc -Xc -B5g5/5g5${TITLE} -W -Ia -Di -Na --MAP_ANNOT_MIN_SPACING=0.5i > $PSFILE.ps

pscmp $PSFILE
