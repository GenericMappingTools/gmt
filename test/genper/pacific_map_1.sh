#!/usr/bin/env bash

ps=pacific_map_1.ps

EARTH_MODEL=e
DEBUG=
X0=-Xc
Y0=-Yc
REGION=-Rg
TITLE=${ps%.ps}
longitude=-140.0
latitude=0.0
altitude=35000.0
tilt=0
azimuth=0
twist=0
Width=0.0
Height=0.0

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

# first generate a gmt grdimage

GRDFILE=@earth_relief_10m
gmt makecpt -Cglobe > t.cpt

gmt grdimage ${GMT_VERBOSE} ${GRDFILE} -P -Xc -Yc -E200 $REGION $PROJ -Ct.cpt -K > $ps
gmt pscoast ${GMT_VERBOSE} $REGION $PROJ -B10g10 -B+t${TITLE} -Ia -Na -Wfaint -O --MAP_ANNOT_MIN_SPACING=0.5i >> $ps

