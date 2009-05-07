#!/bin/sh
#
#	$Id: pscoast_JS.sh,v 1.1 2009-05-07 20:46:41 remko Exp $

ps=pscoast_JS.ps

. ../functions.sh
header "Test pscoast for JS plot for Arctic"

pscoast -JS0/90/7i -R-180/180/65/80 -Dc -A1000 -Gred -Sblue -B30g30f10/g10Sn --BASEMAP_TYPE=PLAIN > $ps

pscmp
