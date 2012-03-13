#!/bin/bash
#	$Id$
#
# Tests project to make oblique lines

. ./functions.sh
header "Test project to make oblique lines"

# Since GMT4 project has no option for small circle we must do it differently
gmtmath -T0/360/1 45 = t.txt
cat << EOF | project -T30/60 -C0/0 -Fpq > p.txt
0 90
0 0
EOF
ppole=`awk '{if (NR == 1) printf "%s/%s\n", $1, $2}' p.txt`
centr=`awk '{if (NR == 2) printf "%s/%s\n", $1, $2}' p.txt`
pscoast -Rg -JG30/50/7i -P -K -Glightgray -Dc -Bg > $ps
echo 30 50 | psxy -R -J -O -K -Sa0.2i -Gred -W0.25p >> $ps
echo 0 0 | psxy -R -J -O -K -Sc0.1i -Gblack >> $ps
project t.txt -T$ppole -C$centr -Fpq | psxy -R -J -O -K -W3p >> $ps
psxy -R -J -O -T >> $ps

pscmp
