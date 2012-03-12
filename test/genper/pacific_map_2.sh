#!/bin/bash
#	$Id$

. functions.sh
header "Test -JG (Pacific 35000 km specific pnt image)"

EARTH_MODEL=e
DEBUG=
COLORMAP="$src"/topo.cpt 
X0=-Xc
Y0=-Yc
REGION=-Rg
TITLE=:.${ps%.ps}:
longitude=-140.0
latitude=0.0
altitude=35000.0
tilt=0
azimuth=0
twist=0
vp_longitude=-160L
vp_latitude=22.5L
Width=2.0
Height=2.0

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${vp_longitude}/${vp_latitude}/${twist}/${Width}/${Height}/7i+

GRDFILE="$src"/etopo2-hawaii.nc

grdimage ${GRDFILE} ${GMT_VERBOSE} -P -Xc -Yc -E200 $REGION $PROJ -C${COLORMAP} -K > $ps
pscoast ${GMT_VERBOSE} $REGION $PROJ -B5g5/5g5${TITLE} -Ia -Na -O --MAP_ANNOT_MIN_SPACING=0.5i >> $ps

pscmp
