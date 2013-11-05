#!/bin/bash
#       $Id$
#
. ./functions.sh

ps=GMT_linecap.ps
cat << EOF > lines.txt
0       0
5       0
EOF

psxy lines.txt -R-0.25/5.25/-0.2/1.4 -Jx1i -P -W4p -K > $ps
psxy lines.txt -R -J -O -K -Y0.2i -W4p,orange,. >> $ps
psxy lines.txt -R -J -O -K -Y0.2i -W4p,red,9_4_2_4:2p >> $ps
psxy lines.txt -R -J -O -K -Y0.2i -W4p,-  --PS_LINE_CAP=round >> $ps
psxy lines.txt -R -J -O -K -Y0.2i -W4p,orange,0_8:0p --PS_LINE_CAP=round >> $ps
psxy lines.txt -R -J -O -K -Y0.2i -W4p,red,0_16:0p  --PS_LINE_CAP=round >> $ps
psxy lines.txt -R -J -O -K -W2p,green,0_16:8p  --PS_LINE_CAP=round >> $ps
psxy lines.txt -R -J -O -T >> $ps
