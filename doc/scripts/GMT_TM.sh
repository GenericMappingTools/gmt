#!/bin/sh
#	$Id: GMT_TM.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R0/360/-80/80 -JT330/-45/3.5i -B30g15/15g15WSne -Dc -A2000 -Gblack -P > GMT_TM.ps
