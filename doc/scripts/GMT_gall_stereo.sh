#!/bin/bash
#	$Id: GMT_gall_stereo.sh,v 1.2 2011-02-28 00:58:03 remko Exp $
#
. functions.sh
gmtset PLOT_DEGREE_FORMAT dddA
pscoast -R-180/180/-60/80 -JCyl_stere/0/45/4.5i -Ba60f30g30/a30g30 -Dc -A5000 -Wblack -Ggrey -P \
	> GMT_gall_stereo.ps
