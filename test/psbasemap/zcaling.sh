#!/usr/bin/env bash
#
# Test that -Jz1:zzzz means 1 plot unit (cm, inch) == zzzz z-units (mGal, degree T, meter)

ps=zscaling.ps

gmt psbasemap -R0/5/0/5/0/8 -Jx1:100 -Jz1:1 -BWSneZ -Baf -Bzaf+l"1 inch == 1 z unit" -p135/5 --PROJ_LENGTH_UNIT=inch -P -K > $ps
gmt psbasemap -R -J -Jz -BWSneZ -Baf -Bzaf+l"1 cm == 1 z unit" -p135/5 -O -X10c --PROJ_LENGTH_UNIT=cm >> $ps
