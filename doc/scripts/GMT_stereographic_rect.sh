#!/bin/bash
#	$Id: GMT_stereographic_rect.sh,v 1.13 2011-06-24 21:13:45 guru Exp $
#
. ./functions.sh

gmtset MAP_ANNOT_OBLIQUE 30
pscoast -R-25/59/70/72r -JS10/90/11c -B20g -Dl -A250 -Gdarkbrown -Wthinnest -P \
	-Slightgray > GMT_stereographic_rect.ps
