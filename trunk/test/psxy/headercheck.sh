#!/bin/bash
#	$Id$
# Test that symbols pick up correct -W -G from command line or header

. ./functions.sh
header "Test psxy and operation of -W -G in headers"

psxy -R-1/10/-1/10 -JX6/4 -P -B2g1 -Sc0.2i -Gyellow -W2.5p,cyan -K << EOF > $ps
> -Ggreen -W1p,black
0	0
1	1
> -Gred	-W-
2	2
3	3
> -Gblue -W
4	4
5	5
> -G-
6	6
7	7
> -G -Wthin,-
8	8
9	9
EOF
#
# Now test that lines/polygons are OK
cat << EOF > tt.cpt
3	p100/9	6	-
6	cyan	9	yellow
EOF
psxy -R -J -O -Y4.75i -Gred -L -B2g1 -Ctt.cpt << EOF >> $ps
> -Ggreen -W
0	0
2	2
0	2
> -G- -W1p,blue
2	0
4	4
3	3
2	0
> -G -W1p,-
0	4
3	6
2	7
> -Gp100/32 -W-
5	0
6	5
4	3
> -W- -Z8
7	4
9	5
8	7
> -W1p -Z5
8	7
9	9
6	9
EOF

pscmp
