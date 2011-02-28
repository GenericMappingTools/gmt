#!/bin/bash
#	$Id: GMT_stereographic_polar.sh,v 1.5 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -R-30/30/60/72 -Js0/90/4.5i/60 -Ba10g5/5g5 -Dl -A250 -Gblack -P > GMT_stereographic_polar.ps
