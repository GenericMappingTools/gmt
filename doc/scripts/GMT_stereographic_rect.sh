#!/bin/bash
#	$Id: GMT_stereographic_rect.sh,v 1.11 2011-06-22 00:35:07 remko Exp $
#
. ./functions.sh

gmtset MAP_ANNOT_OBLIQUE 30
pscoast -R-25/59/70/72r -JS10/90/11c -B30g10/5g -Dl -A250 -Gdarkbrown -Wthinnest -P \
	-Slightgray > GMT_stereographic_rect.ps
