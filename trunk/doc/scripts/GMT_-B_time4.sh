#!/bin/bash
#	$Id: GMT_-B_time4.sh,v 1.7 2011-02-28 00:58:00 remko Exp $
#
. functions.sh
gmtset PLOT_CLOCK_FORMAT -hham ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R0.2t/0.35t/0/1 -JX-5/0.2 -Bpa15mf5m -Bsa1HS -P > GMT_-B_time4.ps
