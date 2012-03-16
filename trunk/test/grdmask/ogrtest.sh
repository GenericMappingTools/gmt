#!/bin/bash
#       $Id$
#
# Test grdmask for proper handling of perimeter and holes [OGR].

header "Test grdmask for OGR perimeter/hole compliance"

# 1. make a regular mask from a perimeter+hole file
grdmask -R77:03:35W/77:03:10W/38:52:05N/38:52:25N -I0.25s -fg -A pentagon.gmt -N0/1/2 -Gmask.nc
cat << EOF > mask.cpt
0	blue	1	-
1	green	2	-
2	red	3	-
EOF
grdimage mask.nc -Cmask.cpt -Jx800id -P -Ba10sf5sWSne -K -Xc > $ps
psxy -Rmask.nc -J -O -K pentagon.gmt -W0.25p,white >> $ps
# 2. make an ID grid from a multipolygon file with IDs set to CPT entries
grdmask -R-3/8/-3/5 -I0.1 -r multihole.gmt -aZ=ID -Ni -GID.nc
cat << EOF > mask.cpt
0	blue	1	- ;B
1	green	2	- ;G
2	red	3	- ;R
3	yellow	4	- ;Y
4	black	5	- ;K
EOF
grdimage ID.nc -Cmask.cpt -Jx0.5i -O -Ba2f1WSne -K -Y5i >> $ps
psxy -RID.nc -J -O -K multihole.gmt -W0.25p,white >> $ps
psscale -Cmask.cpt -D5.75i/2i/2i/0.15i -O -Li0.05i >> $ps

pscmp
