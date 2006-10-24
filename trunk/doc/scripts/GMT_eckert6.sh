#!/bin/sh
#	$Id: GMT_eckert6.sh,v 1.3 2006-10-24 01:53:19 remko Exp $
#

pscoast -R0/360/-90/90 -JK180/4.5i -Bg30/g15 -Dc -A10000 -Wthinnest -Gwhite -Slightgray -P > GMT_eckert6.ps
