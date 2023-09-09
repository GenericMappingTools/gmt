#!/usr/bin/env bash
# Test issue #1204
# Text should be red with blue outline but the quoted text is blue.
ps=outline.ps
cat << EOF > t.txt
0 0.25
1 0.25
EOF
echo "0.5 0.75 Outlined text" | gmt pstext -JX11c -R0/1/0/1 -F+f40p,Times-Bold,red=1p,blue -B0 -P -K -Xc > $ps
gmt psxy t.txt -J -R -W1p -Sqn1:+l"Outlined text"+f40p,Times-Bold,red=1p,blue -O -K >> $ps
echo "0.5 0.75 Outlined text" | gmt pstext -J -R -F+f40p,Times-Bold,red=1p,blue -B0 -O -K -Y12c >> $ps
gmt psxy t.txt -J -R -W1p -Sqn1:+l"Outlined text"+h+f40p,Times-Bold,red=1p,blue -O >> $ps
