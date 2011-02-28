#!/bin/bash
#	$Id: GMT_-B_time1.sh,v 1.10 2011-02-28 00:58:03 remko Exp $
#
. functions.sh
gmtset PLOT_DATE_FORMAT -o ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R2000-4-1T/2000-5-25T/0/1 -JX5/0.2 -Bpa7Rf1d -Bsa1OS -P > GMT_-B_time1.ps
