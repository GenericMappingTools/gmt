#!/bin/sh
#	$Id: GMT_robinson.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

pscoast -R-180/180/-90/90 -JN0/4.5i -Bg30/g15 -Dc -A10000 -G128 -P > GMT_robinson.ps
