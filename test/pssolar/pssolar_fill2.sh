#!/usr/bin/env bash
# Documenting issue # 1155 [Andreas].  The problem is that the terminator
# touches the N pole and what is inside and outside of the global polygon
# becomes harder to decide.  This case shows that while pssolar selects the
# wrong side, dumping it to file and plotting via psxy selects the correct side.
# We are faking the orig PS by using psxy for both, then commenting that out
ps=pssolar_fill2.ps
gmt pscoast -Rd -JKs0/7i -Dc -A50000 -W0.5p -S175/210/255 -Bafg --MAP_FRAME_TYPE=plain -P -K -Xc > $ps

date="2017-09-26T12:40:58"

# Dump to file then plot
gmt pssolar -Ta+d$date -M > t.txt
gmt psxy -R  -J t.txt -Wthick -Gnavy@95 -K -O >> $ps
gmt pssolar -I+d$date -C | gmt psxy -R -J -Sc1c -Gyellow -O -K -B+t"$date dump via psxy" >> $ps

# Draw in pssolar instead
gmt pscoast -R -J -Dc -A50000 -W0.5p -S175/210/255 -Bafg --MAP_FRAME_TYPE=plain -O -K -Y5i >> $ps
#gmt psxy -R  -J t.txt -Wthick -Gnavy@95 -K -O >> $ps
gmt pssolar -R -J -Ta+d$date -Wthick -Gnavy@95 -K -O >> $ps
gmt pssolar -I+d$date -C | gmt psxy -R -J -Sc1c -Gyellow -O -B+t"$date directly via pssolar" >> $ps
