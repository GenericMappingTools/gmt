#!/bin/bash
#	$Id: GMT_-B_time7.sh,v 1.8 2011-02-28 00:58:00 remko Exp $
#
. functions.sh
gmtset PLOT_DATE_FORMAT jjj TIME_INTERVAL_FRACTION 0.05 ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R2000-12-15T/2001-1-15T/0/1 -JX5/0.2 -Bpa5Df1d -Bsa1YS -P > GMT_-B_time7.ps
