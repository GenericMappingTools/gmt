#!/bin/bash
#	$Id: GMT_log.sh,v 1.6 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

psxy -R1/100/0/10 -Jx1.5il/0.15i -B2g3/a2f1g2WSne -Wthick,blue,- -P -K -h sqrt.d > GMT_log.ps
psxy -R -J -Ss0.1i -N -Gred -W -O -h sqrt.d10 >> GMT_log.ps
