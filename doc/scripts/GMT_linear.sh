#!/bin/bash
#	$Id: GMT_linear.sh,v 1.4 2011-02-28 00:58:00 remko Exp $
#
. functions.sh

psxy -R0/100/0/10 -JX3i/1.5i -Ba20f10g10/a2f1g2WSne -Wthick,- -P -K sqrt.d > GMT_linear.ps
psxy -R -J -St0.075i -Glightgray -W -O sqrt.d10 >> GMT_linear.ps
