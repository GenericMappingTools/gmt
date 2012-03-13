#!/bin/bash
#       $Id$
#
# Test psxy for proper handling of -a [OGR].  We read an OGR file
# with depth and magnitude among the aspatial values and we wish to
# use that as input cols 2 and 3, then scale col 3 (mag) by a log10
# transform to get symbol sizes.

. ./functions.sh
header "Test psxy and pstext and their use of aspatial data"

makecpt -Crainbow -T0/300/25 -Z > t.cpt
psxy "$src"/quakes.gmt -R15/25/15/25 -JM6i -B5 -Sci -Ct.cpt -P -K -Wthin -a2=depth,3=magnitude -i0,1,2,3s0.05l -Yc -Xc > $ps
pstext "$src"/quakes.gmt -R -J -O -K -a2=name -F+jCT -Dj0/0.2i >> $ps
psscale -Ct.cpt -D3i/-0.5i/6i/0.1ih -O -B:"Epicenter Depth":/:km: >> $ps

pscmp
