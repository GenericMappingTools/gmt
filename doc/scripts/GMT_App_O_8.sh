#!/bin/bash
#	$Id$
#
#	Makes Fig 8 for Appendix O (labeled lines)
#
gmt gmtconvert -i0,1,4 -Em150 transect.txt | $AWK '{print $1,$2,int($3)}' > fix2.txt
gmt pscoast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500 -K -P > GMT_App_O_8.ps
gmt grdcontour geoid.nc -J -O -K -B20f10 -BWSne -C10 -A20+d+u" m"+f8p -Gl50/10S/160/10S -S10 \
	-T:-+ >> GMT_App_O_8.ps
gmt psxy -R -J -O -Sqffix2.txt:+g+an+p+Lf+u" m"+f8p -Wthick transect.txt >> GMT_App_O_8.ps
