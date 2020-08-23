#!/usr/bin/env bash

ps=east_map_6.ps

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
vp_latitude=39.5855L
vp_longitude=-75.2858L
twist=0
Width=10.0
Height=10.0

# point from an altitude of 160 km at a specific point on the earth and with a restricted view


PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${vp_longitude}/${vp_latitude}/${twist}/${Width}/${Height}/7i+

gmt pscoast ${GMT_VERBOSE} $REGION $PROJ -P -Yc -Xc -B5g5 -B+t${TITLE} -G128/255/128 -S128/128/255 -W -A1000 -Di --MAP_ANNOT_MIN_SPACING=0.5i > $ps
