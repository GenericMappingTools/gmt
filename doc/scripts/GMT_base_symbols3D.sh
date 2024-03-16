#!/usr/bin/env bash
#
# Plot psxyz 3-D symbols

gmt begin GMT_base_symbols3D
    gmt set GMT_THEME cookbook
cat << EOF > x.txt
0	a	-Su
1	a	-SU
2	a	-So
3	a	-SO
4	a	-So+z
EOF
    gmt makecpt -Cjet -T0/4/1
    echo 0 0 0.3 | gmt plot3d -R-0.5/4.5/-0.1/0.5/0/0.8 -Jx1i -Jz1i -W0.25p -Su0.5c -Gred -p160/35 -Bxcx.txt -Byaf -BSlz
    echo 1 0 0.3 | gmt plot3d -W0.25p -SU0.5c -Glightred -p
    echo 2 0 0.75 | gmt plot3d -W0.25p -So0.5c -Gblue -p
    echo 3 0 0.75 | gmt plot3d -W0.25p -SO0.5c -Glightblue -p
    echo 4 0 0.2 0.4 0.5 0.75 | gmt plot3d -W0.25p -So0.5c+z4 -C -p
gmt end show
