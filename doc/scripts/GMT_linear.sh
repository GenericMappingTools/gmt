#!/bin/sh
#	$Id: GMT_linear.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

psxy -R0/100/0/10 -JX3i/1.5i -Ba20f10g10/a2f1g2WSne -W1p,- -P -K sqrt.d > GMT_linear.ps
psxy -R -JX -St0.075i -Glightgray -W -O sqrt.d10 >> GMT_linear.ps
