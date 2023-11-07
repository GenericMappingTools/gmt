#!/usr/bin/env bash
# See issue # 1092.  Fixed in r18275. Old subversion stuff
ps=cpttime.ps
cat << EOF  > tmp.cpt
98 black 100 black
100 white 108 white
EOF
gmt psscale -JX20cT/10c -R2016-04-01T/2016-04-15T/8.1/8.25 -Dn0.5/0.9+w10c/0.8c+jMC+h -B1 -Ctmp.cpt -P > $ps
