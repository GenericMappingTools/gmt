#!/bin/bash
#	$Id$
#
#	Makes Fig 4 for Appendix O (labeled lines)
#
. ./functions.sh

pscoast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500 -K -P > GMT_App_O_4.ps
grdcontour geoid.nc -J -O -B20f10WSne -C10 -A20+d+f8p -GLZ-/Z+ -S10 -T:LH >> GMT_App_O_4.ps
