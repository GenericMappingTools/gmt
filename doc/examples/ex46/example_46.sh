#!/bin/bash
#               GMT EXAMPLE 46
#               $Id$
#
# Purpose:      Illustrate use of pssolar to plot day/night terminators
# GMT modules:  pssolar, pscoast, psxy
#

ps=example_46.ps
gmt pscoast -Rd -JKs0/10i -Dl -A5000 -W0.5p -N1/0.5p,gray -S175/210/255 -Bafg --MAP_FRAME_TYPE=plain -K -Xc > $ps
gmt pssolar -R  -J -Td+d2016-02-09T16:00:00 -Gnavy@95 -K -O >> $ps
gmt pssolar -R  -J -Tc+d2016-02-09T16:00:00 -Gnavy@85 -K -O >> $ps
gmt pssolar -R  -J -Tn+d2016-02-09T16:00:00 -Gnavy@80 -K -O >> $ps
gmt pssolar -R  -J -Ta+d2016-02-09T16:00:00 -Gnavy@80 -K -O >> $ps
gmt pssolar -I+d2016-02-09T16:00:00 -C | gmt psxy -R -J -Sksunglasses/1.5c -Gyellow -O >> $ps
