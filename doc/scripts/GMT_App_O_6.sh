#!/usr/bin/env bash
#
#	Makes Fig 6 for Appendix O (labeled lines)
#
gmt begin GMT_App_O_6
	gmt coast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500
	gmt grdcontour @App_O_geoid.nc -B20f10 -BWSne -C10 -A20+d+f8p -Gl50/10S/160/10S -S10 -T+l
	gmt plot -SqD1000k:+g+LD+an+p -Wthick @App_O_transect.txt
gmt end show
