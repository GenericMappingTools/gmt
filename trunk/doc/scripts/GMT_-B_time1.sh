#!/bin/sh
#	$Id: GMT_-B_time1.sh,v 1.7 2004-04-13 17:37:39 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT -o ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R2000-4-1T/2000-5-25T/0/1 -JX6T/0.2 -Bpa7Rf1d -Bsa1OS -P > GMT_-B_time1.ps
