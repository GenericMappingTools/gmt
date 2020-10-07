#!/usr/bin/env bash
#
# Description:

ps=pssac_F.ps

gmt pssac seis.sac -R9/20/-2/2 -JX15c/5c -Bx1 -By1+l'-Fr' -BWSen -W1p,black -Fr -K -P > $ps
gmt pssac seis.sac -R9/20/-0.06/0.06 -J -Bx1 -By0.02+l'-Fri' -BWsen -W1p,black -Fri -Y6c -K -O >> $ps
gmt pssac seis.sac -R9/20/-8e-3/8e-3 -J -Bx1 -By4e-3+l'-Friri' -BWsen -W1p,black -Friri -Y6c -K -O >> $ps
gmt pssac seis.sac -R9/20/-0.1/3 -J -Bx1 -By1+l'-Frq' -BWsen -W1p,black -Frq -Y6c -O >> $ps
