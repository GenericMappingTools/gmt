#!/bin/sh
#	$Id: GMT_TM.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

pscoast -R0/360/-80/80 -JT330/-45/3.5i -B30g15/15g15WSne -Dc -A2000 -G0 -P > GMT_TM.ps
