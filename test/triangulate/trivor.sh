#!/bin/sh
#	$Id: trivor.sh,v 1.4 2010-07-21 02:57:46 remko Exp $
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
triangulate nodes.xy -m | psxy -m -R0/10/0/10 -JX6 -P -K -W0.25p,red > trivor.ps
psxy -R -J -O -B2g1 -Sc0.2 -Gwhite -W0.25p nodes.xy -K >> trivor.ps
awk '{printf "%s %s 8 0 0 CM %d\n", $1, $2, NR-1}' nodes.xy | pstext -R -J -O -K >> trivor.ps
triangulate nodes.xy -m -Q -R0/10/0/10 | psxy -R0/10/0/10 -J -O -K -W1p -m >> trivor.ps
psxy -R -J -O /dev/null >> trivor.ps

rm -f .gmtcommands4 nodes.xy

pscmp
