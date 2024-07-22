#!/usr/bin/env bash
#
#	Makes Fig 3 for Appendix O (labeled lines)
#
cat << EOF > fix.txt
80      -8.5
55      -7.5
102     0
130     10.5
EOF
gmt begin GMT_App_O_3
	gmt coast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500
	gmt grdcontour @App_O_geoid.nc -B20f10 -BWSne -C10 -A20+d+f8p -Gffix.txt/0.1i -S10 -T+lLH
gmt end show
