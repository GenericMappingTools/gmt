#!/bin/sh
#	$Id: GMT_log.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

psxy -R1/100/0/10 -Jx1.5il/0.15i -B2g3/a2f1g2WSne -W1t2_2:0p -P -K -H sqrt.d > GMT_log.ps
psxy -R -Jx -Ss0.075i -G0 -W -O -H sqrt.d10 >> GMT_log.ps
