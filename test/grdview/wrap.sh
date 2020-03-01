#!/usr/bin/env bash
# GMT_KNOWN_FAILURE
# grdview does not close gap across periodic boundary that is not split, such as for azimuthal projections
# There are three troubles here:
# 1) The gap, which is seen in the output of this script (no orig present to ensure test will fail)
# 2) For -JP, the "pole" 90 is at the horizon, not the center, so the problem with start/stop/inc that
#    I recently fixed for Victor fails for -JP.  We probably need to flip something in set_loop_order for that to work
# 3) The y-placement is wrong so I need to compensate with -Y-3i for the first layer
#
# The script below shows both problems by plotting -JA (only problem 1) and -JP (both problems)
ps=wrap.ps
gmt grdview test.grd -JA0/90/6i -Bg10a30p/g10a10p -JZ3i -pz130/30 -P -Qmpink -K -Y-3.5i > $ps
gmt grdview test.grd -JP6i+t-90  -Bg10a30p/g10a10p -JZ3i -pz130/30 -O -Qmpink -Y5i >> $ps
