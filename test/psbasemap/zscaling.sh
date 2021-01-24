#!/usr/bin/env bash
#
# Test that -Jz1:zzzz always means 1 m == zzzz z-units (mGal, degree T, meter)
# regardless of chosen projection distance unit (cm. inch, point)

ps=zscaling.ps

gmt psbasemap -R0/7/0/7/0/10 -Jx1:100 -Jz1:50 -BWSneZ -Baf -Bzaf+l"1 meter == 50 z unit, H -> 7.87 inch [PROJ_LENGTH_UNIT=inch]" -p175/5 --PROJ_LENGTH_UNIT=inch -P -K > $ps
gmt psbasemap -R -J -Jz -BWSneZ -Baf -Bzaf+l"1 meter == 50 z unit, H -> 20 cm [PROJ_LENGTH_UNIT=cm" -p175/5 -O -X10c --PROJ_LENGTH_UNIT=cm >> $ps
