#!/bin/bash
#	$Id$
# Test -Sqn versus -Sqs
ps=segmentize.ps
cat << EOF > t.txt
0	0
1	0
1	1
0	1
0	0
EOF
gmt psxy -R-0.25/1.25/-0.25/1.25 -JX4i -P -Baf -BWSne t.txt -Sqn2:+l"HELLO" -W1p,red -K -Xc > $ps
echo 0.5 0.5 -Sqn2:+l"HELLO" | gmt pstext -R -J -O -K -F+jCM+f16p >> $ps
gmt psxy -R -J -O -K -Baf -BWSne t.txt -Sqs2:+l"HELLO" -W1p,red -Y5i >> $ps
echo 0.5 0.5 -Sqs2:+l"HELLO" | gmt pstext -R -J -O -F+jCM+f16p >> $ps
