#!/bin/bash
#	$Id$
#
gmt gmtset FORMAT_GEO_MAP dddA
gmt pscoast -R-180/180/-60/80 -JCyl_stere/0/45/4.5i -Bxa60f30g30 -Bya30g30 -Dc -A5000 -Wblack \
	-Gseashell4 -Santiquewhite1 -P > GMT_gall_stereo.ps
