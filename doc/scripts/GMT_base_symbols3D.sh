#!/usr/bin/env bash
#
# Plot psxyz 3-D symbols

ps=GMT_base_symbols3D.ps
cat << EOF > x.txt
0	a	-Su
1	a	-SU
2	a	-So
3	a	-SO
4	a	-So+z
EOF
gmt makecpt -Cjet -T0/4/1 > t.cpt
echo 0 0 0.3 | gmt psxyz -R-0.5/4.5/-0.1/0.5/0/0.8 -Jx1i -Jz1i -P -W0.25p -Su0.5c -Gred -K -p160/35 -Bxcx.txt -Byaf -BSlz > $ps
echo 1 0 0.3 | gmt psxyz -R -Jx -Jz -W0.25p -SU0.5c -Glightred -O -K -p >> $ps
echo 2 0 0.75 | gmt psxyz -R -Jx -Jz -W0.25p -So0.5c -Gblue -O -K -p >> $ps
echo 3 0 0.75 | gmt psxyz -R -Jx -Jz -W0.25p -SO0.5c -Glightblue -O -K -p >> $ps
echo 4 0 0.2 0.4 0.5 0.75 | gmt psxyz -R -Jx -Jz -W0.25p -So0.5c+z4 -Ct.cpt -O -p >> $ps
