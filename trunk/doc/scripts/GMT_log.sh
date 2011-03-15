#!/bin/bash
#	$Id: GMT_log.sh,v 1.5 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

psxy -R1/100/0/10 -Jx1.5il/0.15i -B2g3/a2f1g2WSne -Wthick,blue,- -P -K -h sqrt.d > GMT_log.ps
psxy -R -J -Ss0.1i -N -Gred -W -O -h sqrt.d10 >> GMT_log.ps
