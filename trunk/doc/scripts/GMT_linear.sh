#!/bin/sh
#	$Id: GMT_linear.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

psxy -R0/100/0/10 -JX3i/1.5i -Ba20f10g10/a2f1g2WSne -W1t3_3:0p -P -K sqrt.d > GMT_linear.ps
psxy -R -JX -St0.075i -G200 -W -O sqrt.d10 >> GMT_linear.ps
