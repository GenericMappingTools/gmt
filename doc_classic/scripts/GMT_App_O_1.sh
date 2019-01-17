#!/usr/bin/env bash
#
#	Makes Fig 1 for Appendix O (labeled lines)
#
gmt pscoast -R50/160/-15/15 -JM5.3i -Gburlywood -Sazure -A500 -K -P > GMT_App_O_1.ps
gmt grdcontour @App_O_geoid.nc -J -O -B20f10 -BWSne -C10 -A20+f8p -Gd1.5i -S10 -T+lLH >> GMT_App_O_1.ps
