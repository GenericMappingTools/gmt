#!/bin/sh
#	$Id: GMT_gnomonic.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

pscoast -R0/360/-90/90 -JF-120/35/60/4.5i -Bg15 -Dc -A10000 -G200 -W0.25p -P > GMT_gnomonic.ps
