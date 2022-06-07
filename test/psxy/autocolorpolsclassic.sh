#!/usr/bin/env bash
# Test auto-color sequencing for fill via CPT or color list [classic mode]
# Point here is that the color sequence continues from first to second process

cat << EOF > p.txt
>
0 0
1 0
1 1
0 1
>
1 1
2 1
2 2
1 2
>
2 2
3 2
3 3
2 3
>
3 3
4 3
4 4
3 4
>
4 4
5 4
5 5
4 5
EOF
ps=autocolorpolsclassic.ps
gmt set COLOR_SET red,green,blue
gmt psxy -R-1/6/-1/6 -JX10c p.txt -Gauto -Baf -P -K -X5c > $ps
gmt psxy p.txt -R -J -Gauto@50 -Baf -O -Y12c >> $ps
