#!/bin/sh
#	$Id: GMT_mollweide.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R-180/180/-90/90 -JW0/4.5i -Bg30/g15 -Dc -A10000 -Gblack -P > GMT_mollweide.ps
