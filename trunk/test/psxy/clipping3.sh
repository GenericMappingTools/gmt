#!/bin/bash
#	$Id$
#
# Check clipping of polygons outside region and with dateline

header "Test psxy -JM for clipping polygons straddling dateline"

cat << EOF > a.txt
150 -65
150 -55
170 -55
170 -65
150 -65
EOF
cat << EOF > b.txt
-170 -65
-170 -55
170 -55
170 -65
-170 -65
EOF
psxy a.txt -B5/2WSne -R140/175/-66/-60 -JM10c -W0.5p,red -Ggray -K -P -Xc > $ps
psxy a.txt -B5/2WSne -R -J -W0.5p,green -O -K -Y2.3i >> $ps
psxy b.txt -B5/2WSne -R165/200/-66/-60 -J -W0.5p,red -Ggray -O -K -Y2.3i >> $ps
psxy b.txt -B5/2WSne -R -J -W0.5p,green -O -Y2.3i >> $ps

pscmp
