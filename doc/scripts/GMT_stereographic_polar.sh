#!/bin/bash
#	$Id: GMT_stereographic_polar.sh,v 1.6 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -R-30/30/60/72 -Js0/90/4.5i/60 -Ba10g5/5g5 -Dl -A250 -Groyalblue \
	-Sseashell -P > GMT_stereographic_polar.ps
