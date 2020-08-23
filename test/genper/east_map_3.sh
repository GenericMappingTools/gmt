#!/usr/bin/env bash

ps=east_map_3.ps

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
Width=30.0
Height=30.0

# now point from an altitude of 160 km with a specific tilt and azimuth but with an unrestricted field of view

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

gmt pscoast ${GMT_VERBOSE} $REGION $PROJ -P -Yc -Xc -B5g5 -B+t${TITLE} -G128/255/128 -S128/128/255 -W -A1000 -Di --MAP_ANNOT_MIN_SPACING=0.5i > $ps
