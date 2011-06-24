#!/bin/bash
#	$Id: GMT_stereographic_polar.sh,v 1.10 2011-06-24 21:13:45 guru Exp $
#
. ./functions.sh

pscoast -R-30/30/60/72 -Js0/90/4.5i/60 -B10g -Dl -A250 -Groyalblue \
	-Sseashell -P > GMT_stereographic_polar.ps
