#!/bin/bash
#	$Id: GMT_linear.sh,v 1.10 2011-06-24 03:43:00 guru Exp $
#
. ./functions.sh

psxy -R0/100/0/10 -JX3i/1.5i -BagWSne+gsnow -Wthick,blue,- -P -K sqrt.d > GMT_linear.ps
psxy -R -J -St0.1i -N -Gred -Wfaint -O sqrt.d10 >> GMT_linear.ps
