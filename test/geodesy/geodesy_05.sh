#!/usr/bin/env bash
# The given vector (0,1) has length 1.  So a length of 1 will be mapped to a length on the plot
# For psvelo, a map scale of 1.5i and a vector scale of 1.5i will yield a vector of length 1.5i
# regardless of map projection and region.
ps=geodesy_05.ps
echo 0	0	0	1	0.1	0.1  0 > t.txt
gmt psvelo t.txt -Jm1.5i -R-1/1/-0.5/1.5 -Se1.5i/0.95+f12p -A9p+e -W1.5p,red -P -Bafg1 -BWSne -K -Xc > $ps
echo 0	60	0	1	0.1	0.1  0 > t.txt
gmt psvelo t.txt -Jm1.5i -R-1/1/59.5/61.5 -Se1.5i/0.95+f12p -A9p+e -W1.5p,red -O -Bxafg1 -Byafg0.5 -BWsne -Xc -Y3.25i >> $ps
