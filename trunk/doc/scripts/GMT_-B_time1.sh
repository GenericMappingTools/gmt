#!/bin/sh
#	$Id: GMT_-B_time1.sh,v 1.2 2001-09-26 04:44:48 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT -o ANOT_FONT_SIZE +9p
psbasemap -R2000-4-1T/2000-5-25T/0/1 -JX6T/0.2 -Bi7kIf1Of1dS -P > GMT_-B_time1.ps
