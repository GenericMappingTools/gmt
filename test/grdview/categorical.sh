#!/bin/bash
#
#       $Id$

. ./functions.sh
header "Test grdview for categorical grid plots"

# The cpt
cat << EOF > tt.cpt
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
cat << EOF | xyz2grd -R0/10/50/60 -I5 -Gtt.nc
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
grdview tt.nc -Ctt.cpt -JU31/2.75 -P -B5g5:."-Qs":WSne -Qs -K --MAP_TITLE_OFFSET=-1i > $ps
# Then plot as texture tiles -Ts:
grdview tt.nc -Ctt.cpt -J -O -K -B5g5:."-T":WSne -T -X3.5 --MAP_TITLE_OFFSET=-1i >> $ps
psscale -Ctt.cpt -D3/3/1.5/0.2 -O -K -L0.1i >> $ps
# Then plot as image -Qi100
grdview tt.nc -Ctt.cpt -J -O -K -B5g5:."-Qi100":WSne -Qi100 -X-3.5 -Y5.0 --MAP_TITLE_OFFSET=-1i >> $ps
# Finally plot as texture image -Qt100
grdview tt.nc -Ctt.cpt -J -O -B5g5:."-Qt100":WSne -Qt100 -X3.5 --MAP_TITLE_OFFSET=-1i >> $ps

pscmp
