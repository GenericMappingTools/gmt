#!/bin/bash
#	$Id: GMT_-B_time4.sh,v 1.8 2011-03-15 02:06:29 guru Exp $
#
. functions.sh
gmtset FORMAT_CLOCK_MAP=-hham FONT_ANNOT_PRIMARY +9p
psbasemap -R0.2t/0.35t/0/1 -JX-5i/0.2i -Bpa15mf5m -Bsa1HS -P > GMT_-B_time4.ps
