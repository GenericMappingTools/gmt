#!/bin/bash
#	$Id: east_map_6.sh,v 1.4 2011-03-15 02:06:38 guru Exp $

. ../functions.sh
header "Test -JG (US East Coast 160 km specific point)"

EARTH_MODEL=e
DEBUG=
X0=-Xc
Y0=-Yc
REGION=-Rg
latitude=41.5
longitude=-74.0
altitude=160.0
tilt=55
azimuth=210
vp_latitude=39.5855L
vp_longitude=-75.2858L
twist=0
Width=10.0
Height=10.0

# point from an altitude of 160 km at a specific point on the earth and with a restricted view

PSFILE=east_map_6.${EARTH_MODEL}
TITLE=:.${PSFILE}:

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${vp_longitude}/${vp_latitude}/${twist}/${Width}/${Height}/7i+

pscoast ${GMT_VERBOSE} $REGION $PROJ -P -Yc -Xc -B5g5/5g5${TITLE} -G128/255/128 -S128/128/255 -W -Ia -Di -Na --MAP_ANNOT_MIN_SPACING=0.5i > $PSFILE.ps

pscmp $PSFILE
