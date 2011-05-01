#!/bin/bash
#	$Id: GMT_-B_time4.sh,v 1.9 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh
gmtset FORMAT_CLOCK_MAP=-hham FONT_ANNOT_PRIMARY +9p
psbasemap -R0.2t/0.35t/0/1 -JX-5i/0.2i -Bpa15mf5m -Bsa1HS -P > GMT_-B_time4.ps
