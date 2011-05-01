#!/bin/bash
#	$Id: GMT_-B_time1.sh,v 1.12 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh
gmtset FORMAT_DATE_MAP=-o FONT_ANNOT_PRIMARY +9p
psbasemap -R2000-4-1T/2000-5-25T/0/1 -JX5i/0.2i -Bpa7Rf1d -Bsa1OS -P > GMT_-B_time1.ps
