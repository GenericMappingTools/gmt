#!/bin/bash
#	$Id: GMT_linear.sh,v 1.8 2011-06-22 00:35:06 remko Exp $
#
. ./functions.sh

psxy -R0/100/0/10 -JX3i/1.5i -Ba20f10g10/a2gf1WSne+gsnow -Wthick,blue,- -P -K sqrt.d > GMT_linear.ps
psxy -R -J -St0.1i -N -Gred -Wfaint -O sqrt.d10 >> GMT_linear.ps
