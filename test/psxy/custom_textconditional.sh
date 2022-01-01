#!/usr/bin/env bash
#
# Test custom symbol with condition test on words

ps=custom_textconditional.ps
cat << EOF > location.txt
160	0	A	CWT	C
180	15	A	CNT	B
-160	0	D	CET	C
180	-15	A	CST	C
EOF
cat << EOF > ifelse0.def
N: 1 s
if \$t0 == A then {
 0 0 1 c -W3p,red
} else {
 0 0 1 c -W5p,green
}
EOF
cat << EOF > ifelse2.def
N: 1 s
if \$t2 == C then {
 0 0 1 c -W3p,red
} else {
 0 0 1 c -W5p,green
}
EOF
gmt psxy -R60/300/-60/60 -JM16c -Baf location.txt -Skifelse0/2c -P -K > $ps
awk '{print $1, $2, $3}' location.txt | gmt pstext -R -J -O -K -F+f14p >> $ps
gmt psxy -R -J -Baf location.txt -Skifelse2/2c -O -K -Y12c >> $ps
awk '{print $1, $2, $5}' location.txt | gmt pstext -R -J -O -F+f14p >> $ps
