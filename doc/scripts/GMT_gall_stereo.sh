#!/bin/bash
#	$Id: GMT_gall_stereo.sh,v 1.4 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh
gmtset FORMAT_GEO_MAP dddA
pscoast -R-180/180/-60/80 -JCyl_stere/0/45/4.5i -Ba60f30g30/a30g30 -Dc -A5000 -Wblack -Ggrey -P \
	> GMT_gall_stereo.ps
