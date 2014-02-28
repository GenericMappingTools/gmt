#!/bin/bash
#	$Id$
#
# Test how gmt psxy handles polygons that wrap around periodic boundaries
# testpol.d is a nasty polygon that exceeds 360-degree range.

ps=wrapping.ps

gmt psxy -Rg -JH180/3i -Bg30 -Gred testpol.d -P -K > $ps
gmt psxy -R -J -W0.25p testpol.d -O -K >> $ps
gmt psxy -Rg -JQ110/3i -B60g30 -BEwSn -Gred testpol.d -O -K -X3.5i >> $ps
gmt psxy -R -J -W0.25p testpol.d -O -K >> $ps
#
gmt psxy -Rg -JH0/3i -Bg30 -Gred testpol.d -O -K -X-3.5i -Y2.3i >> $ps
gmt psxy -R -J -W0.25p testpol.d -O -K >> $ps
gmt psxy -Rg -JI30/3i -Bg30 -Gred testpol.d -O -K -X3.5i >> $ps
gmt psxy -R -J -W0.25p testpol.d -O -K >> $ps
#
gmt psxy -R0/360/-90/90 -JX3/1.5i -B60g30 -BWSne -Gred testpol.d -O -K -X-3.5i -Y2.3i >> $ps
gmt psxy -R -J -W0.25p testpol.d -O -K >> $ps
gmt psxy -Rg -JA180/0/2i -Bg30 -Gred testpol.d -O -K -X4i -Y-0.25i >> $ps
gmt psxy -R -J -W0.25p testpol.d -O -K >> $ps
#
gmt psxy -R-220/220/-90/90 -JX6.5/1.75i -B60g30 -BWSne -Gred testpol.d -O -K -X-4i -Y2.7i >> $ps
gmt psxy -R -J -W0.25p testpol.d -O -K >> $ps
gmt psxy -R -J -T -O >> $ps

