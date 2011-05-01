#!/bin/bash
#	$Id: GMT_stereographic_rect.sh,v 1.10 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

gmtset MAP_ANNOT_OBLIQUE 30
pscoast -R-25/59/70/72r -JS10/90/11c -B30g10/5g5 -Dl -A250 -Gdarkbrown -Wthinnest -P \
	-Slightgray > GMT_stereographic_rect.ps
