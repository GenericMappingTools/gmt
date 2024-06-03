#!/usr/bin/env bash
#
#	Makes Fig 2 for Appendix O (labeled lines)
#
gmt begin GMT_App_O_2
	gmt coast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500
	gmt grdcontour @App_O_geoid.nc -B20f10 -BWSne -C10 -A20+f8p -Gn1/1i -S10 -T+lLH
gmt end show
