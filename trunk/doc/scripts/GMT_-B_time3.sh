#!/bin/sh
#	$Id: GMT_-B_time3.sh,v 1.6 2006-10-30 19:09:11 remko Exp $
#
gmtset PLOT_DATE_FORMAT o TIME_FORMAT_PRIMARY Character
psbasemap -R1997T/1999T/0/1 -JX5/0.2 -Bpa3Of1o -Bsa1YS -P > GMT_-B_time3.ps
