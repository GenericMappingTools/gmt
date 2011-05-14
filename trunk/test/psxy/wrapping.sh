#!/bin/bash
#	$Id: wrapping.sh,v 1.12 2011-05-14 00:04:07 guru Exp $
#
# Test how psxy handles polygons that wrap around periodic boundaries
# testpol.d is a nasty polygon that exceeds 360-degree range.

. ../functions.sh
header "Test psxy and wrapping of polygons in 0-360"

ps=wrapping.ps
psxy -Rg -JH180/3i -B0g30 -Gred testpol.d -P -K > $ps
psxy -R -J -W0.25p testpol.d -O -K >> $ps
psxy -Rg -JQ110/3i -B60g30EwSn -Gred testpol.d -O -K -X3.5i >> $ps
psxy -R -J -W0.25p testpol.d -O -K >> $ps
#
psxy -Rg -JH0/3i -B0g30 -Gred testpol.d -O -K -X-3.5i -Y2.3i >> $ps
psxy -R -J -W0.25p testpol.d -O -K >> $ps
psxy -Rg -JI30/3i -B0g30 -Gred testpol.d -O -K -X3.5i >> $ps
psxy -R -J -W0.25p testpol.d -O -K >> $ps
#
psxy -R0/360/-90/90 -JX3/1.5i -B60g30WSne -Gred testpol.d -O -K -X-3.5i -Y2.3i >> $ps
psxy -R -J -W0.25p testpol.d -O -K >> $ps
psxy -Rg -JA180/0/2i -B0g30 -Gred testpol.d -O -K -X4i -Y-0.25i >> $ps
psxy -R -J -W0.25p testpol.d -O -K >> $ps
#
psxy -R-220/220/-90/90 -JX6.5/1.75i -B60g30WSne -Gred testpol.d -O -K -X-4i -Y2.7i >> $ps
psxy -R -J -W0.25p testpol.d -O -K >> $ps
psxy -R -J -T -O >> $ps

pscmp
