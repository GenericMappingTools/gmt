#!/bin/bash
#	$Id: GMT_pow.sh,v 1.6 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

psxy -R0/100/0/10 -Jx0.3ip0.5/0.15i -Ba1p/a2f1WSne -Wthick -P -K sqrt.d > GMT_pow.ps
psxy -R -J -Sc0.075i -Gwhite -W -O sqrt.d10 >> GMT_pow.ps
