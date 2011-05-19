#!/bin/bash
#	$Id: GMT_App_O_3.sh,v 1.9 2011-05-19 14:45:45 remko Exp $
#
#	Makes Fig 3 for Appendix O (labeled lines)
#
. ./functions.sh

cat << EOF > fix.d
80      -8.5
55      -7.5
102     0
130     10.5
EOF
pscoast -R50/160/-15/15 -JM5.3i -Glightgray -A500 -K -P > GMT_App_O_3.ps
grdcontour geoid.nc -J -O -B20f10WSne -C10 -A20+d+f8p -Gffix.d/0.1i -S10 -T:LH >> GMT_App_O_3.ps
