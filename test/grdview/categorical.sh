#!/usr/bin/env bash
#

ps=categorical.ps

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
cat << EOF | gmt xyz2grd -R0/10/50/60 -I5 -Gtt.nc
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
# First plot as normal image using gmt surface -Qs:
gmt grdview tt.nc -Ctt.cpt -JU31/2.75 -P -B5g5 -BWSne+t"-Qs" -Qs -K --MAP_TITLE_OFFSET=-1i > $ps
# Then plot as texture tiles -Ts:
gmt grdview tt.nc -Ctt.cpt -J -O -K -B5g5 -BWSne+t"-T" -T -X3.5 --MAP_TITLE_OFFSET=-1i >> $ps
gmt psscale -Ctt.cpt -Dx3/3+w1.5/0.2+jML -L0.1i -O -K >> $ps
# Then plot as image -Qi100
gmt grdview tt.nc -Ctt.cpt -J -O -K -B5g5 -BWSne+t"-Qi100" -Qi100 -X-3.5 -Y5.0 --MAP_TITLE_OFFSET=-1i >> $ps
# Finally plot as texture image -Qt100
gmt grdview tt.nc -Ctt.cpt -J -O -B5g5 -BWSne+t"-Qt100" -Qt100 -X3.5 --MAP_TITLE_OFFSET=-1i >> $ps

