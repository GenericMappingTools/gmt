#!/bin/bash
#	$Id$

. ../functions.sh
header "Test -JG (US East Coast 160 km image)"

EARTH_MODEL=e
DEBUG=
COLORMAP=topo.cpt 
X0=-Xc
Y0=-Yc
REGION=-Rg
latitude=41.5
longitude=-74.0
altitude=160.0
tilt=55
azimuth=210
twist=0
Width=30.0
Height=30.0
PSFILE=east_map_8
TITLE=:.${PSFILE}:

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

GRDFILE=etopo2-chesapeake.nc

grdimage ${GMT_VERBOSE} ${GRDFILE} -P -Xc -Yc -E200 $REGION $PROJ -C${COLORMAP} -K > $PSFILE.ps
pscoast ${GMT_VERBOSE} $REGION $PROJ -B5g5/5g5${TITLE} -Ia -Na -O --MAP_ANNOT_MIN_SPACING=0.5i >> $PSFILE.ps

pscmp $PSFILE
