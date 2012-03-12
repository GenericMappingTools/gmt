#!/bin/bash
#	$Id$
#
. ./functions.sh

. $src/GMT_dummydata.sh

psxy -R0/100/0/10 -JX3i/1.5i -BagWSne+gsnow -Wthick,blue,- -P -K sqrt.d > GMT_linear.ps
psxy -R -J -St0.1i -N -Gred -Wfaint -O sqrt.d10 >> GMT_linear.ps
