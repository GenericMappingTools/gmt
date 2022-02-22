#!/usr/bin/env bash
ps=east_map_8.ps

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

gmt makecpt -Cglobe > t.cpt
gmt grdcut -R275/290/30/45 @earth_relief_02m -Getopo2-chesapeake.nc=ns
gmt grdimage ${GMT_VERBOSE} etopo2-chesapeake.nc -P -Xc -Yc $REGION $PROJ -Ct.cpt -K > $ps
gmt pscoast ${GMT_VERBOSE} $REGION $PROJ -B5g5 -B+t${TITLE} -Ia -Na -O --MAP_ANNOT_MIN_SPACING=0.5i >> $ps
