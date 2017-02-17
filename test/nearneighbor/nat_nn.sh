#!/bin/bash
#	$Id$
#
# Test Delauney and Voronoi for Cartesian data

ps=nat_nn.ps

cat << EOF > tridata.txt
2.53857	5.16657	0
2.48365	6.26811	1
8.68883	4.55983	2
4.11104	7.78623	3
1.79704	6.53027	4
7.17493	3.81713	5
3.41052	8.18161	6
8.35062	1.43348	7
8.1706	4.46765	8
5.27815	1.15172	9
EOF

gmt makecpt -Ccategorical -T0/10/1 > t.cpt
gmt nearneighbor tridata.txt -Nn -R0/10/0/10 -Gjunk.nc -I0.05
gmt grdimage junk.nc -JX6i -P -Baf -BWSnE+t"Natural Nearest Neighbor Gridding" -Ct.cpt -Xc > $ps
