#!/bin/sh
#	$Id: GMT_pow.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

psxy -R0/100/0/10 -Jx0.3ip0.5/0.15i -Ba1p/a2f1WSne -W1p -P -K sqrt.d > GMT_pow.ps
psxy -R -Jx -Sc0.075i -Gwhite -W -O sqrt.d10 >> GMT_pow.ps
