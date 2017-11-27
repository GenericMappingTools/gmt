#!/bin/bash
#	$Id$
#
# Plot bars symbols (horizontal and vertical) w/wo. base
# Plots differ as the given x or y measures total height relative to base
# or relative to origin (i.e., absolute height).

ps=barshv.ps

cat << EOF > tt.d
1	1
2	2
3	3
4	4
5	5
EOF
gmt psxy -R0/7/0/7 -JX3i -P -B0g1 -Sb0.2i+b0.5 -Gblue -W0.25p -X1i -Y1i tt.d -K > $ps
gmt psxy -R -J -O -B0g1 -Sb0.2i+B0.5 -Gblue -W0.25p -X3.25i tt.d -K >> $ps
gmt psxy -R -J -O -B0g1 -SB0.2i+b0.5 -Gblue -W0.25p -X-3.25i -Y3.5i tt.d -K >> $ps
gmt psxy -R -J -O -B0g1 -SB0.2i+B0.5 -Gblue -W0.25p -X3.25i tt.d >> $ps
