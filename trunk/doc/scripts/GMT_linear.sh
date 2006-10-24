#!/bin/sh
#	$Id: GMT_linear.sh,v 1.3 2006-10-24 01:53:19 remko Exp $
#

psxy -R0/100/0/10 -JX3i/1.5i -Ba20f10g10/a2f1g2WSne -Wthick,- -P -K sqrt.d > GMT_linear.ps
psxy -R -J -St0.075i -Glightgray -W -O sqrt.d10 >> GMT_linear.ps
