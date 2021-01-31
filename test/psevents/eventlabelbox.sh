#!/usr/bin/env bash
# Test gmt psevents -H -D event labeling with some colored circles

ps=eventlabelbox.ps

gmt math -T1/9/1 T = | awk '{printf "%s\t%s\t%s\t%s\tEvent Label %s\n", $1, $1, $1, $1, $1}' > e.txt
gmt makecpt -Cjet -T0/10 > q.cpt
gmt psevents -R0/11/0/10 -Jx1.5c -Baf -BWSrt e.txt -Dj3c+v1p,red -F+f16p+jLM -Sc1c -Wthin -Cq.cpt -T6.5 -L1 -H+gpink+pthin+c25%+s+r -Es+r2+d6 -Et -Ms5+c0.5 -Mi1+c-0.6 -Mt+c0 -P > $ps
