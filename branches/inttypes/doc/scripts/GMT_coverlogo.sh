#!/bin/bash
#	$Id$
#
# Creates the cover page GMT logo
#
#	Logo is 5.458" wide and 2.729" high and origin is lower left
#
scale=2.729
psxy -R0/2/0/1 -Jx${scale}i -P -K -X0 -Y0 -T > GMT_coverlogo.ps
gmtlogo 0 0 $scale >> GMT_coverlogo.ps
psxy -R -J -O -T >> GMT_coverlogo.ps
