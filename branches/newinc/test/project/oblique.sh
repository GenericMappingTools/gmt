#!/bin/bash
#	$Id$
#
# Tests gmt project to make oblique lines

ps=oblique.ps

# Since GMT4 gmt project has no option for small circle we must do it differently
gmt gmtmath -T0/360/1 45 = t.txt
cat << EOF | gmt project -T30/60 -C0/0 -Fpq > p.txt
0 90
0 0
EOF
ppole=`$AWK '{if (NR == 1) printf "%s/%s\n", $1, $2}' p.txt`
centr=`$AWK '{if (NR == 2) printf "%s/%s\n", $1, $2}' p.txt`
gmt pscoast -Rg -JG30/50/7i -P -K -Glightgray -Dc -Bg > $ps
echo 30 50 | gmt psxy -R -J -O -K -Sa0.2i -Gred -W0.25p >> $ps
echo 0 0 | gmt psxy -R -J -O -K -Sc0.1i -Gblack >> $ps
gmt project t.txt -T$ppole -C$centr -Fpq | gmt psxy -R -J -O -K -W3p >> $ps
gmt psxy -R -J -O -T >> $ps

