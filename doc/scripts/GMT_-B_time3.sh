#!/bin/sh
#	$Id: GMT_-B_time3.sh,v 1.3 2004-04-12 21:41:47 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT o TIME_FORMAT_SECONDARY Character
psbasemap -R1997T/1999T/0/1 -JX6T/0.2 -Bpa1YS -Bsa3Of1o -P > GMT_-B_time3.ps
