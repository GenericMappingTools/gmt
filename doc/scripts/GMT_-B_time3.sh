#!/bin/sh
#	$Id: GMT_-B_time3.sh,v 1.4 2004-04-13 17:37:39 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT o TIME_FORMAT_PRIMARY Character
psbasemap -R1997T/1999T/0/1 -JX6T/0.2 -Bpa3Of1o -Bsa1YS -P > GMT_-B_time3.ps
