#!/usr/bin/env bash

ps=hyperbolic_map.ps

EARTH_MODEL=e
DEBUG=
X0=-Xc
Y0=-Yc
REGION=-Rg
TITLE=${ps%.ps}
latitude=41.5
longitude=-74.0
altitude=160.0
tilt=55
azimuth=210
twist=0
Width=0.0
Height=0.0

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

gmt pscoast ${GMT_VERBOSE} $REGION $PROJ -P -Yc -Xc -B5g5 -B+t${TITLE} -G128/255/128 -S128/128/255 -W -Di --MAP_ANNOT_MIN_SPACING=0.5i > $ps
