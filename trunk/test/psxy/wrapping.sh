#!/bin/sh
#	$Id: wrapping.sh,v 1.1 2006-03-10 07:24:56 pwessel Exp $
#
# Test how psxy handles polygons that wrap around periodic boundaries
# testpole is a nasty polygon that exceeds 360-degree range.

psxy -Rg -JH180/3i -B0g30 -Gred testpol.d -P -K > wrap.ps
psxy -R -J -W0.25p testpol.d -O -K >> wrap.ps
psxy -Rg -JQ110/3i -B60g30EwSn -Gred testpol.d -O -K -X3.5i >> wrap.ps
psxy -R -J -W0.25p testpol.d -O -K >> wrap.ps
#
psxy -Rg -JH0/3i -B0g30 -Gred testpol.d -O -K -X-3.5i -Y2.5i >> wrap.ps
psxy -R -J -W0.25p testpol.d -O -K >> wrap.ps
psxy -Rg -JI30/3i -B0g30 -Gred testpol.d -O -K -X3.5i >> wrap.ps
psxy -R -J -W0.25p testpol.d -O -K >> wrap.ps
#
psxy -R0/360/-90/90 -JX3/1.5i -B60g30WSne -Gred testpol.d -O -K -X-3.5i -Y2.5i >> wrap.ps
psxy -R -J -W0.25p testpol.d -O -K >> wrap.ps
psxy -Rg -JA180/0/1.53i -B0g30 -Gred testpol.d -O -K -X3.5i >> wrap.ps
psxy -R -J -W0.25p testpol.d -O -K >> wrap.ps
#
psxy -R-220/220/-90/90 -JX6/1.5i -B60g30WSne -Gred testpol.d -O -K -X-3.5i -Y2.5i >> wrap.ps
psxy -R -J -W0.25p testpol.d -O -K >> wrap.ps
psxy -R -J /dev/null -O >> wrap.ps
gv wrap.ps &
