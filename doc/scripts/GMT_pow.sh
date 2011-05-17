#!/bin/bash
#	$Id: GMT_pow.sh,v 1.7 2011-05-17 03:02:06 guru Exp $
#
. ./functions.sh

psxy -R0/100/0/10 -Jx0.3ip0.5/0.15i -Ba1p/a2f1WSne -Wthick -P -K sqrt.d > GMT_pow.ps
psxy -R -J -Sc0.075i -Ggreen -W -O sqrt.d10 >> GMT_pow.ps
