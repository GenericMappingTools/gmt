#!/bin/sh
#	$Id: GMT_gall_stereo.sh,v 1.1 2008-04-28 17:44:43 remko Exp $
#
gmtset PLOT_DEGREE_FORMAT dddA
pscoast -R-180/180/-60/80 -JCyl_stere/0/45/4.5i -Ba60f30g30/a30g30 -Dc -A5000 -Wblack -Ggrey -P \
	> GMT_gall_stereo.ps
