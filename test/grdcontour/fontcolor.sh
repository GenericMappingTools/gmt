#!/usr/bin/env bash
# Testing issue #1125.  Should be fixed and this is the test.
ps=fontcolor.ps
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
gmt grdcontour -A+1+c50%+f36p,Times-Bold,red -Gl0/0/2/2 test.grd -JX6i -Baf -BseWN -P -Xc -Wa1p,blue > $ps
