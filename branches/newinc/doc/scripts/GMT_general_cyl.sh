#!/bin/bash
#	$Id$
#
gmt pscoast -R-145/215/-90/90 -JY35/30/4.5i -B45g45 -Dc -A10000 -Sdodgerblue -Wthinnest -P \
	--MAP_FRAME_TYPE=fancy+ > GMT_general_cyl.ps
