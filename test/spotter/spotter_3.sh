#!/bin/bash
#
#       $Id$

ps=spotter_3.ps

# Example 3 - Using gmt originator
#
# We will use gmt originator to determine the most likely hotspot origins
# for the seamounts in the seamounts.d file, given a plate motion model
# and a list of possible hotspots.

POLES=${src}/../../src/spotter/WK97.d # Rotation poles to use
pac_ps=${src}/../../src/spotter/pac_hs.d

grep -v '^#' ${pac_ps} > tmp
$AWK '{printf "s/%s/%d/g\n", $3, NR}' tmp > t.sed
gmt makecpt -Ccategorical -T1/11/1 -N > t.cpt
paste t.cpt tmp | $AWK '{printf "%s\t%s\t;%s\n", $1, $2, $5}' > key.cpt
gmt originator ${src}/../../src/spotter/seamounts.d -S1 -D10m -E${POLES} -F${pac_ps} | sed -f t.sed > spotter_3.d
gmt pscoast -R130/260/-55/60 -JM6i -P -K -G30/120/30 -A500 -Dl -W0.25p -B20 -BWSne -Xc -Y1.5i > $ps
gmt psxy spotter_3.d -R -J -O -K -Ckey.cpt -i0,1,5 -Sc0.03i >> $ps
grep -v '^#' ${pac_ps} | sed -f t.sed | gmt psxy -R -J -O -K -Sa0.2i -W0.25p -Ckey.cpt >> $ps
grep -v '^#' ${pac_ps} | cut -f1,2,3 | gmt pstext -R -J -O -K -F+jCT+f10p -Dj0.15i/0.15i -Gwhite -W0.5p >> $ps
gmt psscale -Ckey.cpt -D3i/-0.5i+w5i/0.14i+h+jTC -O -Li0.15i >> $ps
