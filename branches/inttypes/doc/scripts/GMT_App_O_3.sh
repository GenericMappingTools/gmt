#!/bin/bash
#	$Id$
#
#	Makes Fig 3 for Appendix O (labeled lines)
#
cat << EOF > fix.d
80      -8.5
55      -7.5
102     0
130     10.5
EOF
pscoast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500 -K -P > GMT_App_O_3.ps
grdcontour geoid.nc -J -O -B20f10WSne -C10 -A20+d+f8p -Gffix.d/0.1i -S10 -T:LH >> GMT_App_O_3.ps
