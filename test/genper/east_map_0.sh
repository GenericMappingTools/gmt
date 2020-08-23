#!/usr/bin/env bash
# Test of gmt projection

ps=east_map_0.ps

EARTH_MODEL=e
DEBUG=
X0=-Xc
Y0=-Yc
REGION=-Rg
TITLE=${ps%.ps}
latitude=41.5
longitude=-74.0
altitude=20000.0
tilt=0
azimuth=0
twist=0
Width=0.0
Height=0.0

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

gmt pscoast $REGION $PROJ -P -Yc -Xc -B10g10 -B+t${TITLE} -G128/255/128 -S128/128/255 -W -A10000 -Dc --MAP_ANNOT_MIN_SPACING=0.5i > $ps
