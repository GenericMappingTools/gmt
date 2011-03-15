#!/bin/bash
#	$Id: GMT_gall_stereo.sh,v 1.3 2011-03-15 02:06:29 guru Exp $
#
. functions.sh
gmtset FORMAT_GEO_MAP dddA
pscoast -R-180/180/-60/80 -JCyl_stere/0/45/4.5i -Ba60f30g30/a30g30 -Dc -A5000 -Wblack -Ggrey -P \
	> GMT_gall_stereo.ps
