#!/bin/sh
#	$Id: GMT_log.sh,v 1.3 2006-10-24 01:53:19 remko Exp $
#

psxy -R1/100/0/10 -Jx1.5il/0.15i -B2g3/a2f1g2WSne -Wthick,- -P -K -H sqrt.d > GMT_log.ps
psxy -R -J -Ss0.075i -Gblack -W -O -H sqrt.d10 >> GMT_log.ps
