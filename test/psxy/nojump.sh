#!/bin/bash
# Added after issue #672 which exposed a map jump that should not happen
# Fixed in r14052 but we keep this test in case it resurfaces
ps=nojump.ps
cat << EOF > t.txt
115 -20
115 15
195 15
195 -20
115 -20
EOF
gmt psxy -R110/215/-27/20 -JM6i t.txt -Wfat,green -Baf -BWSne -A -P -K -Xc > $ps
gmt psxy -R -J t.txt -Wfat,green -Baf -BWSne -Ap -O -K -Y3.15i >> $ps
gmt psxy -R -J t.txt -Wfat,green -Baf -BWSne -O -Y3.15i >> $ps
