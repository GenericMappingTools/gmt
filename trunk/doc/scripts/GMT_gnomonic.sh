#!/bin/sh
#	$Id: GMT_gnomonic.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R0/360/-90/90 -JF-120/35/60/4.5i -Bg15 -Dc -A10000 -Glightgray -W0.25p -P > GMT_gnomonic.ps
