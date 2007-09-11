#!/bin/sh
#
#       $Id: categorical.sh,v 1.2 2007-09-11 22:56:12 remko Exp $

ps=categorical.ps
echo -n "$0: Test grdview for categorical grid plots:		"
# The cpt
cat << EOF > $$.cpt
1	red	2	red	;A
2	green	3	green	;B
3	blue	4	blue	;C
4	yellow	5	yellow	;D
5	brown	6	brown	;E
B	black
F	white
N	gray
EOF
# The categorical data grid
cat << EOF | xyz2grd -R0/10/50/60 -I5 -G$$.grd
0       60      1
5       60      1
10      60      2
0       55      1
5       55      2
10      55      2
0       50      2
5       50      5
10      50      5
EOF
#
# First plot as normal image using surface -Qs:
grdview $$.grd -C$$.cpt -JU31/2.75 -P -B5g5:."-Qs":WSne -Qs -K --HEADER_OFFSET=-1i > $ps
# Then plot as texture tiles -Ts:
grdview $$.grd -C$$.cpt -J -O -K -B5g5:."-T":WSne -T -X3.5 --HEADER_OFFSET=-1i >> $ps
psscale -C$$.cpt -D3/3/1.5/0.2 -O -K -L0.1i >> $ps
# Then plot as image -Qi100
grdview $$.grd -C$$.cpt -J -O -K -B5g5:."-Qi100":WSne -Qi100 -X-3.5 -Y5.0 --HEADER_OFFSET=-1i >> $ps
# Finally plot as texture image -Qt100
grdview $$.grd -C$$.cpt -J -O -B5g5:."-Qt100":WSne -Qt100 -X3.5 --HEADER_OFFSET=-1i >> $ps

rm -f $$.* .gmtcommands4

compare -density 100 -metric PSNR {,orig/}$ps categorical_diff.png > log 2>&1
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
        echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail categorical_diff.png log
fi
