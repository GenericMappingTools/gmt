#!/bin/bash
#	$Id$
#
#	Makes Fig 5 for Appendix O (labeled lines)
#
. ./functions.sh

pscoast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500 -K -P > GMT_App_O_5.ps
grdcontour "$src"/geoid.nc -J -O -B20f10WSne -C10 -A20+d+f8p -GX"$src"/cross.d -S10 -T:LH >> GMT_App_O_5.ps
