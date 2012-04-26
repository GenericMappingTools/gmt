#!/bin/bash
#	$Id$

ps=pacific_map_1.ps

EARTH_MODEL=e
DEBUG=
COLORMAP="${src:=.}"/topo.cpt 
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
Width=0.0
Height=0.0

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

# first generate a grdimage

GRDFILE=etopo10.nc

grdimage ${GMT_VERBOSE} ${GRDFILE} -P -Xc -Yc -E200 $REGION $PROJ -C${COLORMAP} -K > $ps
pscoast ${GMT_VERBOSE} $REGION $PROJ -B10g10/10g10${TITLE} -Ia -Na -O --MAP_ANNOT_MIN_SPACING=0.5i >> $ps

