#!/bin/bash
#
#       $Id$

. ./functions.sh
header "Testing originator to determine nearest origin"

# Example 3 - Using originator
#
# We will use originator to determine the most likely hotspot origins
# for the seamounts in the seamounts.d file, given a plate motion model
# and a list of possible hotspots.

POLES="$src"/WK97.d			# Rotation poles to use

grep -v '^#' "$src"/pac_hs.d > tmp
awk '{printf "s/%s/%d/g\n", $3, NR}' tmp > t.sed
grep -v '^#' "$src"/pac_hs.d | awk '{printf "s/%s/%d/g\n", $3, NR}' > t.sed
makecpt -Ccategorical -T1/11/1 -N > t.cpt
paste t.cpt tmp | awk '{printf "%s\t%s\t;%s\n", $1, $2, $5}' > key.cpt
originator "$src"/seamounts.d -S1 -D10m -E${POLES} -F"$src"/pac_hs.d | sed -f t.sed > spotter_3.d
pscoast -R130/260/-55/60 -JM6i -P -K -G30/120/30 -A500 -Dl -W0.25p -B20WSne -Xc -Y1.5i > $ps
psxy spotter_3.d -R -J -O -K -Ckey.cpt -i0,1,5 -Sc0.03i >> $ps
grep -v '^#' "$src"/pac_hs.d | sed -f t.sed | psxy -R -J -O -K -Sa0.2i -W0.25p -Ckey.cpt >> $ps
grep -v '^#' "$src"/pac_hs.d | cut -f1,2,3 | pstext -R -J -O -K -F+jCT+f10p -Dj0.15i/0.15i -Gwhite -W0.5p >> $ps
psscale -Ckey.cpt -D3i/-0.5i/5i/0.14ih -O -Li0.15i >> $ps

pscmp
