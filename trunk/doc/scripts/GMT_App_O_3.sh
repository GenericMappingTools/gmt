#!/bin/bash
#	$Id: GMT_App_O_3.sh,v 1.6 2011-02-28 00:58:00 remko Exp $
#
#	Makes Fig 3 for Appendix O (labeled lines)
#
. functions.sh

cat << EOF > fix.d
80      -8.5
55      -7.5
102     0
130     10.5
EOF
pscoast -R50/160/-15/15 -JM5.3i -Glightgray -A500 -K -P > GMT_App_O_3.ps
grdcontour geoid.nc -J -O -B20f10WSne -C10 -A20+d+s8 -Gffix.d/0.1i -S10 -T:LH >> GMT_App_O_3.ps
