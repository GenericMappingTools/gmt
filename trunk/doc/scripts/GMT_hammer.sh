#!/bin/sh
#	$Id: GMT_hammer.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R0/360/-90/90 -JH180/4.5i -Bg30/g15 -Dc -A10000 -Gblack -P > GMT_hammer.ps
