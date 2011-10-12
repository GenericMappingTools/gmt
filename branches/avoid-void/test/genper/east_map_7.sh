#!/bin/bash
#	$Id$

. ../functions.sh
header "Test -JG (US East Coast 20000 km image)"

EARTH_MODEL=e
DEBUG=
COLORMAP=topo.cpt 
X0=-Xc
Y0=-Yc
REGION=-Rg
PSFILE=east_map_7
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

# first generate a grdimage

GRDFILE=etopo10.nc

grdimage ${GMT_VERBOSE} ${GRDFILE} -P -Xc -Yc -E200 $REGION $PROJ -C${COLORMAP} -K > $PSFILE.ps
pscoast ${GMT_VERBOSE} $REGION $PROJ -B10g10/10g10${TITLE} -Ia -Na -O --MAP_ANNOT_MIN_SPACING=0.5i >> $PSFILE.ps

pscmp $PSFILE
