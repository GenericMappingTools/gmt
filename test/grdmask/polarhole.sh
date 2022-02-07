#!/usr/bin/env bash
# Test grdmask with search radius around some points
# Made because of http://gmt.soest.hawaii.edu/boards/1/topics/5379
ps=polarhole.ps
# 31 unit radius Cartesian with periodic BC in x
gmt grdmask -Gmask_c.grd -I1 -R0/360/-90/90 -N1/1/NaN -S31 -n+bpx << EOF
0	39
EOF
# 31 degree radius geographic
gmt grdmask -Gmask_g.grd -I1 -R0/360/-90/90 -N1/1/NaN -S31d << EOF
0	39
EOF
gmt makecpt -Cjet -T0/10 > t.cpt
gmt grdimage mask_c.grd -R-180/180/-90/90 -JX5.5id/2.75id -Ct.cpt -Baf -BWSnE -P -K -Xc -Y0.75i --MAP_FRAME_TYPE=plain > $ps
gmt grdimage mask_c.grd -R0/360/0/80 -JP2.75i+a -Ct.cpt -Bafg -O -K -X-0.75i -Y3.25i >> $ps
gmt grdimage mask_g.grd -R0/360/0/80 -JP2.75i+a -Ct.cpt -Bafg -O -K -X3.75i >> $ps
gmt grdimage mask_g.grd -JQ0/5.5i -Ct.cpt -Baf -O -X-3i -Y3.5i >> $ps
