#!/bin/sh
#	$Id: GMT_-B_time5.sh,v 1.1 2001-09-25 01:09:26 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT u
psbasemap -R2001-9-11T/2001-9-29T/0/1 -JX6t/0.2 -Bic1KI1US -P -K > GMT_-B_time5.ps
gmtset PLOT_DATE_FORMAT o TIME_WEEK_START Sunday
psbasemap -R2001-9-8T/2001-9-29T/0/1 -JX6t/0.2 -Bf1kic3KI1rS -O -Y0.65i >> GMT_-B_time5.ps
