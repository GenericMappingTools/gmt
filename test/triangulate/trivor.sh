#!/bin/bash
#	$Id: trivor.sh,v 1.6 2011-05-14 00:04:07 guru Exp $
#
# Test Delauney and Voronoi for Cartesian data

. ../functions.sh
header "Test triangulate for generating Delaunay and Voronoi edges"

cat << EOF > nodes.xy
2.53857	5.16657
2.48365	6.26811
8.68883	4.55983
4.11104	7.78623
1.79704	6.53027
7.17493	3.81713
3.41052	8.18161
8.35062	1.43348
8.1706	4.46765
5.27815	1.15172
EOF

ps=trivor.ps
triangulate nodes.xy -M | psxy -R0/10/0/10 -JX6 -P -K -W0.25p,red > $ps
psxy -R -J -O -B2g1 -Sc0.2 -Gwhite -W0.25p nodes.xy -K >> $ps
awk '{printf "%s %s %d\n", $1, $2, NR-1}' nodes.xy | pstext -R -J -F+f8p -O -K >> $ps
triangulate nodes.xy -M -Q -R0/10/0/10 | psxy -R0/10/0/10 -J -O -K -W1p >> $ps
psxy -R -J -O -T >> $ps

rm -f nodes.xy

pscmp
