#!/bin/bash
#	$Id$
#
#	Makes Fig 1 for Appendix O (labeled lines)
#
pscoast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500 -K -P > GMT_App_O_1.ps
grdcontour geoid.nc -J -O -B20f10WSne -C10 -A20+f8p -Gd1.5i -S10 -T:LH >> GMT_App_O_1.ps
