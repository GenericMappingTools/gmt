#!/usr/bin/env bash
#
# Testing absolute gravity crossovers from c2308 near Oahu.

ps=x2sys_06.ps

export X2SYS_HOME=`pwd`
gmt x2sys_init LINE -D"${src:-.}"/line -Etxt -G -F
gmt x2sys_cross -TLINE @c2308.txt -Qi > c2308_faa_x.txt
gmt grdmath -R199/204/18/25 -I5m -fg c2308_faa_x.txt PDIST = dist_km.grd
gmt makecpt -Chot -T0/100 > t.cpt
gmt math c2308_faa_x.txt -i10 ABS = xfaa.txt
gmt grdimage dist_km.grd -Ct.cpt -JM6i -P -K -Xc -Ei > $ps
gmt pscoast -R -J -O -Ggray -Baf -K -Df >> $ps
gmt psxy -R -J @c2308.txt -W0.25p,cyan -O -K -i0,1 >> $ps
gmt psxy c2308_faa_x.txt -R -J -K -O -Sc0.02c -Ggreen@50 >> $ps
gmt psbasemap -R -J -O -K -DjTL+w2i+o0.6i/0.2i+t >> $ps
gmt pshistogram xfaa.txt -W2 -R0/150/0/30 -JX2i -Gred -L0.25p -BWSne+glightblue -Byaf+u% -Bxaf+lmGal -O -K >> $ps
gmt psxy -R -J -O -T >> $ps
