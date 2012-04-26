#!/bin/bash
#	$Id$
#
psxy -R0/100/0/10 -Jx0.3ip0.5/0.15i -Ba1p/a2f1WSne+givory -Wthick -P -K sqrt.d > GMT_pow.ps
psxy -R -J -Sc0.075i -Ggreen -W -O sqrt.d10 >> GMT_pow.ps
