#!/usr/bin/env bash
#
# Test gmt grdmask for proper handling of perimeter and holes [OGR].

ps=ogrtest.ps

# 1. make a regular mask from a perimeter+hole file
gmt grdmask -R77:03:35W/77:03:10W/38:52:05N/38:52:25N -I0.25s -fg -A @pentagon.gmt -N0/1/2 -Gmask.nc
cat << EOF > mask.cpt
0	blue	1	-
1	green	2	-
2	red	3	-
EOF
gmt grdimage mask.nc -Cmask.cpt -Jx800id -P -Ba10sf5s -BWSne -K -Xc > $ps
gmt psxy -Rmask.nc -J -O -K @pentagon.gmt -W0.25p,white >> $ps
# 2. make an ID grid from a multipolygon file with IDs set to CPT entries
gmt grdmask -R-3/8/-3/5 -I0.1 -r @multihole.gmt -aZ=ID -Nz -GID.nc -fg
cat << EOF > mask.cpt
0	blue	1	- ;B
1	green	2	- ;G
2	red	3	- ;R
3	yellow	4	- ;Y
4	black	5	- ;K
EOF
gmt grdimage ID.nc -Cmask.cpt -Jx0.5i -O -Ba2f1 -BWSne -K -Y5i >> $ps
gmt psxy -RID.nc -J -O -K @multihole.gmt -W0.25p,white >> $ps
gmt psscale -Cmask.cpt -Dx5.75i/2i+w2i/0.15i+jML -O -Li0.05i >> $ps

