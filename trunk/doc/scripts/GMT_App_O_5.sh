#!/bin/sh
#	$Id: GMT_App_O_5.sh,v 1.3 2004-06-24 05:02:06 pwessel Exp $
#
#	Makes Fig 5 for Appendix O (labeled lines)
#

pscoast -R50/160/-15/15 -JM5.5i -Glightgray -A500 -K -P > GMT_App_O_5.ps
grdcontour geoid.grd -JM -O -B20f10WSne -C10 -A20+d+s8 -GXcross.d -S10 -T:LH >> GMT_App_O_5.ps
