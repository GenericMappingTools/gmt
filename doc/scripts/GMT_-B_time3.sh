#!/bin/sh
#	$Id: GMT_-B_time3.sh,v 1.2 2004-04-12 19:51:12 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT o TIME_ANNOT_FORMAT2 Character
psbasemap -R1997T/1999T/0/1 -JX6T/0.2 -Bpa1YS -Bsa3Of1o -P > GMT_-B_time3.ps
