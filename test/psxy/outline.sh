#!/bin/bash
# Test issue #1204
ps=outline.ps
echo "0.5 0.75 Outlined text" | gmt pstext -JX15c -R0/1/0/1 -F+f40p,Times-Bold,red=1p,blue -B0 -P -K > $ps
cat << EOF | gmt psxy -J -R -Sqn1:+l"Outlined text"+f40p,Times-Bold,red=1p,blue -O >> $ps
0 0.25
1 0.25
EOF
