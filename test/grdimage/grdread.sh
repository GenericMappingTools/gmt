#!/bin/bash
#
# Demonstrate netcdf and binary native grid reading
#
#	$Id$

ps=grdread.ps
$AWK 'BEGIN{n=12;m=0;for (j=0;j<6*n;j++) {if (j%n==0) m--;print sin(m*3.14159265/6);m++}}' | \
	gmt xyz2grd -R15/345/-75/75 -I30 -Gtmp.nc -ZTLa -fg
gmt grdreformat tmp.nc tmp.b=bf
gmt makecpt -Crainbow -T-1/1/0.1 > tmp.cpt
# Read netCDF grid
gmt grdimage -Rd -Ctmp.cpt tmp.nc -JX4i/2i -Bx60f10 -By30f10 -BWeSn --MAP_FRAME_TYPE=plain --FONT_ANNOT_PRIMARY=10p --FORMAT_GEO_MAP=DF -P -K -Y8.25i -Xc > $ps
gmt grdimage -R-165/195/-90/90 -Ctmp.cpt tmp.nc -J -Bx60f10 -By30f10 -BWeSn --MAP_FRAME_TYPE=plain --FONT_ANNOT_PRIMARY=10p --FORMAT_GEO_MAP=DF -O -K -Y-2.5i >> $ps
# Read native binary grid
gmt grdimage -Rd -Ctmp.cpt tmp.b=bf -J -Bx60f10 -By30f10 -BWeSn --MAP_FRAME_TYPE=plain --FONT_ANNOT_PRIMARY=10p --FORMAT_GEO_MAP=DF -O -K -Y-2.5i >> $ps
gmt grdimage -R-165/195/-90/90 -Ctmp.cpt tmp.b=bf -J -Bx60f10 -By30f10 -BWeSn --MAP_FRAME_TYPE=plain --FONT_ANNOT_PRIMARY=10p --FORMAT_GEO_MAP=DF -O -K -Y-2.5i >> $ps
gmt psscale -Ctmp.cpt -D4.5i/4.5i/8i/0.2i -O -K >> $ps
gmt pstext -R0/9/0/9 -Jx1 -O -N -F+f18p+jCM << EOF >> $ps
-1.2 7.25 NetCDF
-1.2 2.25 Binary
EOF
