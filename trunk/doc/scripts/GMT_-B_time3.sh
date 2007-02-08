#!/bin/sh
#	$Id: GMT_-B_time3.sh,v 1.7 2007-02-08 21:46:27 remko Exp $
#
gmtset PLOT_DATE_FORMAT o TIME_FORMAT_PRIMARY Character ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R1997T/1999T/0/1 -JX5/0.2 -Bpa3Of1o -Bsa1YS -P > GMT_-B_time3.ps
