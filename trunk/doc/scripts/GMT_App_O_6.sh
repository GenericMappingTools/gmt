#!/bin/bash
#	$Id: GMT_App_O_6.sh,v 1.11 2011-05-01 18:06:37 remko Exp $
#
#	Makes Fig 6 for Appendix O (labeled lines)
#
. ./functions.sh

pscoast -R50/160/-15/15 -JM5.3i -Glightgray -A500 -K -P > GMT_App_O_6.ps
grdcontour geoid.nc -J -O -K -B20f10WSne -C10 -A20+d+s8 -Gl50/10S/160/10S -S10 \
	-T:'-+' >> GMT_App_O_6.ps
psxy -R -J -O -SqD1000k:+g+LD+an+p -Wthick transect.d >> GMT_App_O_6.ps
