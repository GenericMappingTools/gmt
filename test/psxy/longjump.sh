#!/bin/bash

# Testing to see if points very far outside the box will be
# a) ignored if geographic
# b) considered if Cartesian
# See rectclip.sh for more background on this issue
# as well as issue # 657

ps=longjump.ps

cat << EOF > t.txt
-100 -100
100 100
EOF
#
gmt psxy -R0/10/0/10 -JX10c -Bafg -W2p,red -P -Xc -K t.txt > $ps
gmt psxy -R -JM10c -Bafg -W2p,red -O -Y12c t.txt >> $ps
