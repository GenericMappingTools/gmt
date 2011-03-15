#!/bin/bash
#	$Id: GMT_-B_time1.sh,v 1.11 2011-03-15 02:06:29 guru Exp $
#
. functions.sh
gmtset FORMAT_DATE_MAP=-o FONT_ANNOT_PRIMARY +9p
psbasemap -R2000-4-1T/2000-5-25T/0/1 -JX5i/0.2i -Bpa7Rf1d -Bsa1OS -P > GMT_-B_time1.ps
