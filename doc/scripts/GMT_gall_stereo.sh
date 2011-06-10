#!/bin/bash
#	$Id: GMT_gall_stereo.sh,v 1.5 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh
gmtset FORMAT_GEO_MAP dddA
pscoast -R-180/180/-60/80 -JCyl_stere/0/45/4.5i -Ba60f30g30/a30g30 -Dc -A5000 -Wblack -Gseashell4 \
	-Santiquewhite1 -P > GMT_gall_stereo.ps
