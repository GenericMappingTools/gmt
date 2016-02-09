#!/bin/bash
#               GMT EXAMPLE 46
#               $Id $
#
# Purpose:      Illustrate use of pssolar to plot day/night terminators
# GMT progs:    pssolar, pscoast, psxy
#

ps=example_46.ps

pscoast -Rd -JX14dc/0c -Dl -A5000 -W0.1p -Slightblue -Bwsen --MAP_FRAME_TYPE=plain -P -K > $ps
pssolar -R  -JX -Td+d2016-02-09T16:00:00 -G150@80 -K -O >> $ps
pssolar -R  -JX -Tc+d2016-02-09T16:00:00 -G150@80 -K -O >> $ps
pssolar -R  -JX -Tn+d2016-02-09T16:00:00 -G150@60 -K -O >> $ps
pssolar -R  -JX -Ta+d2016-02-09T16:00:00 -G150@60 -K -O >> $ps
echo -56.4477 -14.7067 | psxy -R -JX -Sc0.4c -Gyellow -O >> $ps
