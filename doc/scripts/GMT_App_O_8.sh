#!/usr/bin/env bash
#
#	Makes Fig 8 for Appendix O (labeled lines)
#
gmt begin GMT_App_O_8
	gmt convert -i0,1,4 -Em150 @App_O_transect.txt | $AWK '{print $1,$2,int($3)}' > fix2.txt
	gmt coast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500
	gmt grdcontour @App_O_geoid.nc -B20f10 -BWSne -C10 -A20+d+u" m"+f8p -Gl50/10S/160/10S -S10 -T+l
	gmt plot -Sqffix2.txt:+g+an+p+Lf+u" m"+f8p -Wthick @App_O_transect.txt
gmt end show
