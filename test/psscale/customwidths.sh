#!/usr/bin/env bash
#
# Test the -Zwidthfile option
# Widths may be a mix of absolute widths with units or just fractionals
# If a mix then all are considered relative amounts.
ps=customwidths.ps
cat << EOF > tmp.cpt
1.00  0/0/205       2.00  0/0/205     L
2.00  0/7/252       4.00  0/7/252     L
4.00  4/129/252     6.00  4/129/252   L
6.00  19/205/253    8.00  19/205/253  L
8.00  31/253/250    10.0  31/253/25   L
10.0  55/253/210    20.0  55/253/210  L
20.0  174/254/91    40.0  174/254/91  L
40.0  255/209/42    60.0  255/209/42  L
60.0  254/137/35    80.0  254/137/35  L
80.0  254/87/32     100.  254/87/32   L
100.  254/53/31     200.  254/53/31   B
EOF
cat << EOF > widths.txt
0.30103c
0.30103i
0.176091
0.124939
0.1i
24p
0.30103
0.176091
0.124939
0.09691
0.30103
EOF
gmt psscale -Ctmp.cpt -Dx2c/11c+w15c/0.5c+jML -K -Zwidths.txt -P  > $ps
gmt psscale -Ctmp.cpt -Dx6c/11c+w20c/0.5c+jML -O -K -Zwidths.txt >> $ps
gmt math widths.txt 3.76 MUL = tmp.txt
gmt psscale -Ctmp.cpt -Dx10c/11c+w15c/0.5c+jML -O -K -Ztmp.txt  >> $ps
gmt psscale -Ctmp.cpt -Dx14c/11c+w20c/0.5c+jML -O -Ztmp.txt     >> $ps
