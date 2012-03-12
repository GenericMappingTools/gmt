#!/bin/bash
#	$Id$
#
#	Makes Fig 2 for Appendix O (labeled lines)
#
. ./functions.sh

pscoast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500 -K -P > GMT_App_O_2.ps
grdcontour "$src"/geoid.nc -J -O -B20f10WSne -C10 -A20+f8p -Gn1/1i -S10 -T:LH >> GMT_App_O_2.ps
