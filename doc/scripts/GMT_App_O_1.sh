#!/bin/sh
#	$Id: GMT_App_O_1.sh,v 1.3 2007-02-08 21:46:28 remko Exp $
#
#	Makes Fig 1 for Appendix O (labeled lines)
#

pscoast -R50/160/-15/15 -JM5.3i -Glightgray -A500 -K -P > GMT_App_O_1.ps
grdcontour geoid.grd -J -O -B20f10WSne -C10 -A20+s8 -Gd1.5i -S10 -T:LH >> GMT_App_O_1.ps
