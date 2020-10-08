#!/usr/bin/env bash
#
# Test Delauney and Voronoi for Cartesian data

ps=trivor.ps

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

gmt triangulate nodes.xy -M | gmt psxy -R0/10/0/10 -JX6i -P -K -W0.25p,red > $ps
gmt psxy -R -J -O -B2g1 -Sc0.2i -Gwhite -W0.25p nodes.xy -K >> $ps
$AWK '{printf "%s %s %d\n", $1, $2, NR-1}' nodes.xy | gmt pstext -R -J -F+f8p -O -K >> $ps
gmt triangulate nodes.xy -M -Q -R0/10/0/10 | gmt psxy -R0/10/0/10 -J -O -W1p >> $ps
