#!/bin/bash
#	$Id: GMT_-B_time7.sh,v 1.10 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh
gmtset FORMAT_DATE_MAP jjj TIME_INTERVAL_FRACTION 0.05 FONT_ANNOT_PRIMARY +9p
psbasemap -R2000-12-15T/2001-1-15T/0/1 -JX5i/0.2i -Bpa5Df1d -Bsa1YS -P > GMT_-B_time7.ps
