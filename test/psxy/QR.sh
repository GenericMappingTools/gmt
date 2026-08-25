#!/usr/bin/env bash
# Test opaque and transparent QR codes in psxy
ps=QR.ps
cat << EOF > t.txt
> -Gred -Wfaint
1 1
> -Gpink -W2p
2 1
> -Gyellow -W-
3 1
> -Gblue -W1p,red
1 2
> -Ggreen -W1p,green
2 2
> -Gcyan -W-
3 2
> -Ggray
1 3
> -Gblack -W1p,white
2 3
> -Gbrown -Wfaint
3 3
EOF

gmt psxy -R0/4/0/4 -JX4i -P -B0 -B+glightgray+t"QR (opaque)" t.txt -Gred -SkQR/0.75i -Xc -K > $ps
gmt psxy -R -J -O -B0 -B+glightgray+t"QR (transparent)" t.txt -Gred -SkQR_transparent/0.75i -Y4.75i >> $ps
