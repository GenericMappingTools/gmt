#!/bin/sh
#	$Id: GMT_App_O_2.sh,v 1.3 2004-06-24 02:07:50 pwessel Exp $
#
#	Makes Fig 2 for Appendix O (labeled lines)
#

pscoast -R50/160/-15/15 -JM5.5i -Glightgray -A500 -K -P > GMT_App_O_2.ps
grdcontour geoid.grd -J -O -B20f10WSne -C10 -A20+s8 -Gn1/1i -S10 -T:LH >> GMT_App_O_2.ps
