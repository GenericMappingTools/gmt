#!/bin/sh
#	$Id: GMT_App_O_7.sh,v 1.2 2004-06-13 20:23:02 pwessel Exp $
#
#	Makes Fig 7 for Appendix O (labeled lines)
#

PS=`basename $0 .sh`
pscoast -R50/160/-15/15 -JM5i -Glightgray -A500 -K -P > $PS.ps
grdcontour geoid.grd -J -O -K -B20f10WSne -C10 -A20+d+um -Gl50/10S/160/10S -S10 -T:-+ >> $PS.ps
psxy -R -J -O -SqD15d:+gblack+kwhite+Ld+o+u-\\312 -W1p transect.d >> $PS.ps
