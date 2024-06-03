#!/usr/bin/env bash
#
#	Makes Fig 5 for Appendix O (labeled lines)
#
gmt begin GMT_App_O_5
	gmt coast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500
	gmt grdcontour @App_O_geoid.nc -B20f10 -BWSne -C10 -A20+d+f8p -GX@App_O_cross.txt -S10 -T+lLH
gmt end show
