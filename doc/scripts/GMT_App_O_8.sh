#!/bin/sh
#	$Id: GMT_App_O_8.sh,v 1.4 2004-06-13 22:30:18 pwessel Exp $
#
#	Makes Fig 8 for Appendix O (labeled lines)
#

awk '{if (NR > 1 && ($3 % 1500) == 0) print $1, $2, int($5)}' transect.d > fix2.d
pscoast -R50/160/-15/15 -JM5.5i -Glightgray -A500 -K -P > GMT_App_O_8.ps
grdcontour geoid.grd -J -O -K -B20f10WSne -C10 -A20+d+um+s8 -Gl50/10S/160/10S -S10 -T:-+ >> GMT_App_O_8.ps
psxy -R -J -O -Sqffix2.d:+g+an+p+Lf+um+s8 -W1p transect.d >> GMT_App_O_8.ps
