#!/bin/bash
#	$Id: GMT_stereographic_rect.sh,v 1.12 2011-06-23 01:17:06 remko Exp $
#
. ./functions.sh

gmtset MAP_ANNOT_OBLIQUE 30
pscoast -R-25/59/70/72r -JS10/90/11c -B30g10/5g5 -Dl -A250 -Gdarkbrown -Wthinnest -P \
	-Slightgray > GMT_stereographic_rect.ps
