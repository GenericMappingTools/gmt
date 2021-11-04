#!/usr/bin/env bash
#
# Show asymmetrical error bars both from deviations (+a) and bounds (+A)
ps=error_bars_asymm.ps

printf "1 20 -3 2\n2 35 -6 3\n4 10 -1 9" | gmt psxy -R0.5/5.5/0/40 -JX16c/10c -Ey+a+p1p -Gred -Sc0.2c -Ba -P -K > $ps
printf "1 20 17 22\n2 35 29 38\n4 10 9 19" | gmt psxy -R -J -Ey+A+p1p -Gred -Sc0.2c -Ba -O -Y12c >> $ps
