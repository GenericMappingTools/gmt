#!/bin/sh
#	$Id: GMT_eckert4.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R0/360/-90/90 -JKf180/4.5i -Bg30/g15 -Dc -A10000 -W0.25p -Gwhite -Slightgray -P > GMT_eckert4.ps
