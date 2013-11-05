#!/bin/bash
#	$Id$
#
. ./functions.sh
gmtset FORMAT_DATE_MAP u FORMAT_TIME_PRIMARY_MAP Character FORMAT_TIME_SECONDARY_MAP full \
	FONT_ANNOT_PRIMARY +9p
psbasemap -R1969-7-21T/1969-8-9T/0/1 -JX5i/0.2i -Bpa1K -Bsa1US -P -K > GMT_-B_time5.ps
gmtset FORMAT_DATE_MAP o TIME_WEEK_START Sunday FORMAT_TIME_SECONDARY_MAP Chararacter
psbasemap -R -J -Bpa3Kf1k -Bsa1rS -O -Y0.65i >> GMT_-B_time5.ps
