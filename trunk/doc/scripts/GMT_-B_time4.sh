#!/bin/sh
#	$Id: GMT_-B_time4.sh,v 1.6 2007-02-08 21:46:27 remko Exp $
#
gmtset PLOT_CLOCK_FORMAT -hham ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R0.2t/0.35t/0/1 -JX-5/0.2 -Bpa15mf5m -Bsa1HS -P > GMT_-B_time4.ps
