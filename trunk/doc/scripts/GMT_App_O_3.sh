#!/bin/sh
#	$Id: GMT_App_O_3.sh,v 1.2 2004-06-13 22:30:18 pwessel Exp $
#
#	Makes Fig 3 for Appendix O (labeled lines)
#

pscoast -R50/160/-15/15 -JM5.5i -Glightgray -A500 -K -P > GMT_App_O_3.ps
grdcontour geoid.grd -J -O -B20f10WSne -C10 -A20+d+s8 -Gffix.d/0.1i -S10 -T:LH >> GMT_App_O_3.ps
