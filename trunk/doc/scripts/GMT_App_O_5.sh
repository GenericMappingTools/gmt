#!/bin/sh
#	$Id: GMT_App_O_5.sh,v 1.2 2004-06-13 22:30:18 pwessel Exp $
#
#	Makes Fig 5 for Appendix O (labeled lines)
#

PS=`basename $0 .sh`
pscoast -R50/160/-15/15 -JM5.5i -Glightgray -A500 -K -P > GMT_App_O_5.ps
grdcontour geoid.grd -JM -O -B20f10WSne -C10 -A20+d+s8 -GXcross.d -S10 -T:LH >> GMT_App_O_5.ps
