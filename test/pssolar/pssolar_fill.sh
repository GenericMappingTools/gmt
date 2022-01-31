#!/usr/bin/env bash
# Documenting issue # 1155 [Andreas].  The problem is that the terminator
# touches the N pole and what is inside and outside of the global polygon
# becomes harder to decide.  The 11:30:58 time is ok but an hour later...
# DVC_TEST
ps=pssolar_fill.ps
gmt pscoast -P -Rd -JKs0/6i -Dc -A50000 -W0.5p -S175/210/255 -Bafg --MAP_FRAME_TYPE=plain -K -Xc > $ps

date="2017-09-26T11:40:58"

gmt pssolar -R  -J -Td+d$date -Wthick -Gnavy@95 -K -O >> $ps
gmt pssolar -R  -J -Tc+d$date -Wthick -Gnavy@85 -K -O >> $ps
gmt pssolar -R  -J -Tn+d$date -Wthick,red@80 -Gnavy@80 -K -O >> $ps
gmt pssolar -R  -J -Ta+d$date -Wthick,blue@80 -Gnavy@80 -K -O >> $ps
gmt pssolar -I+d$date -C | gmt psxy -R -J -Sc1c -Gyellow -O -K -B+t"$date" >> $ps

gmt pscoast -R -J -Dc -A50000 -W0.5p -S175/210/255 -Bafg --MAP_FRAME_TYPE=plain -O -K -Y4.5i >> $ps

date="2017-09-26T12:40:58"

gmt pssolar -R  -J -Td+d$date -Wthick -Gnavy@95 -K -O >> $ps
gmt pssolar -R  -J -Tc+d$date -Wthick -Gnavy@85 -K -O >> $ps
gmt pssolar -R  -J -Tn+d$date -Wthick,red@80 -Gnavy@80 -K -O >> $ps
gmt pssolar -R  -J -Ta+d$date -Wthick,blue@80 -Gnavy@80 -K -O >> $ps
gmt pssolar -I+d$date -C | gmt psxy -R -J -Sc1c -Gyellow -O -B+t"$date" >> $ps
