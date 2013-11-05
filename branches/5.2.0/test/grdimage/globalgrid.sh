#!/bin/bash
#
# Check if we can wrap global grids over longitude
#
#	$Id$

ps=globalgrid.ps

plot="gmt grdimage -Ctmp.cpt tmp.nc -JX3i/1.5i -Bx60f10 -By30f10 -BWeSn --MAP_FRAME_TYPE=plain --FONT_ANNOT_PRIMARY=10p --FORMAT_GEO_MAP=DF"
#
makegrid ()
{
	$AWK 'BEGIN{n=12;m=0;for (j=0;j<6*n;j++) {if (j%n==0) m--;print sin(m*3.14159265/6);m++}}' | \
	gmt xyz2grd -I30 -Gtmp.nc -ZTLa -fg $*
}

R1=-Rg
R2=-Rd
R3=-R15/360/-90/90
R4=-R-165/195/-90/90

gmt makecpt -Crainbow -T-1/1/0.1 > tmp.cpt
makegrid -R0/360/-90/90 -r
$plot $R1 -Y6.5i -K > $ps
$plot $R2 -Y-2i -O -K >> $ps
$plot $R3 -Y-2i -O -K >> $ps
$plot $R4 -Y-2i -O -K >> $ps
makegrid -R15/345/-75/75
$plot $R1 -Y6i -X5i -O -K >> $ps
$plot $R2 -Y-2i -O -K >> $ps
$plot $R3 -Y-2i -O -K >> $ps
$plot $R4 -Y-2i -O >> $ps

