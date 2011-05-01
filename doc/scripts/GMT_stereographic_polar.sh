#!/bin/bash
#	$Id: GMT_stereographic_polar.sh,v 1.7 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -R-30/30/60/72 -Js0/90/4.5i/60 -Ba10g5/5g5 -Dl -A250 -Groyalblue \
	-Sseashell -P > GMT_stereographic_polar.ps
