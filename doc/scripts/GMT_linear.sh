#!/bin/bash
#	$Id: GMT_linear.sh,v 1.5 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

psxy -R0/100/0/10 -JX3i/1.5i -Ba20f10g10/a2f1g2WSne -Wthick,blue,- -P -K sqrt.d > GMT_linear.ps
psxy -R -J -St0.1i -N -Gred -Wfaint -O sqrt.d10 >> GMT_linear.ps
