#!/usr/bin/env bash
# DVC_TEST

ps=pacific_map_2.ps

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
vp_longitude=-160L
vp_latitude=22.5L
Width=2.0
Height=2.0

PROJ=-JG${DEBUG}${EARTH_MODEL}${longitude}/${latitude}/${altitude}/${vp_longitude}/${vp_latitude}/${twist}/${Width}/${Height}/7i+

gmt makecpt -Cglobe > t.cpt
gmt grdcut @earth_relief_02m -R189/210/10/33 -Getopo2-hawaii.nc=ns
gmt grdimage etopo2-hawaii.nc ${GMT_VERBOSE} -P -Xc -Yc $REGION $PROJ -Ct.cpt -K > $ps
gmt pscoast ${GMT_VERBOSE} $REGION $PROJ -B5g5 -B+t${TITLE} -Wfaint -O --MAP_ANNOT_MIN_SPACING=0.5i >> $ps
