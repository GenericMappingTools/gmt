#!/usr/bin/env bash
# Due to hairline differences in many gridlines between Linux and macOS we need a
# higher rms threshold for this test to pass
# GRAPHICSMAGICK_RMS = 0.0052

ps=snyder_map.ps

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

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${azimuth}/${tilt}/${twist}/${Width}/${Height}/7i+

gmt pscoast ${GMT_VERBOSE} $REGION $PROJ -P -Yc -Xc -B5g1 -B+t${TITLE} -W -Ia -Di -Na --MAP_ANNOT_MIN_SPACING=0.5i > $ps
