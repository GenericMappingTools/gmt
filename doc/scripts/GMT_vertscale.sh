#!/bin/bash
#	$Id$
#
ps=GMT_vertscale.ps
echo 0 0 0 > t.txt
gmt pswiggle -R-10/10/-3/3 -JM6i -Baf -Z100i -DjRM+l100+unT t.txt -Gred -BWSne -P --MAP_FRAME_TYPE=plain > $ps
