#!/usr/bin/env bash
# Address issue #1126.  Should be fixed.
ps=transparency.ps
gmt xyz2grd -R0/2/0/2 -I1/1 -Gtest.grd << EOF
0 0 0
1 0 1
2 0 0
0 1 1
1 1 2
2 1 1
0 2 0
1 2 1
2 2 0
EOF
echo "0 red 2 blue" > test.cpt
gmt grdimage test.grd -Ctest.cpt -JX6i -Baf -BseWN -P -K -Xc > $ps
gmt grdcontour -A+1+c50%+f36p,Times-Bold,white@50+o+p2p,green@20+gblack@60 -Gl0/0/2/2 test.grd -J -W2p,magenta@100 -O >> $ps
