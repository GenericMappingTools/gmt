#!/bin/sh
#
#	$Id: globalgrid.sh,v 1.9 2009-06-12 02:09:09 remko Exp $

ps=globalgrid.ps
#
# Check if we can wrap global grids over longitude
#
. ../functions.sh
header "Test grdimage for wrapping global grids"
plot=grdimage" -Ctmp.cpt tmp.nc -JX3i/1.5i -B60f10/30f10WeSn --BASEMAP_TYPE=plain --ANNOT_FONT_SIZE_PRIMARY=10p --PLOT_DEGREE_FORMAT=DF"
#
makegrid ()
{
	awk 'BEGIN{n=12;m=0;for (j=0;j<6*n;j++) {if (j%n==0) m--;print sin(m*3.14159265/6);m++}}' | \
	xyz2grd -I30 -Gtmp.nc -ZTLa -fg $*
}

R1=g
R2=d
R3=15/360/-90/90
R4=-165/195/-90/90

makecpt -Crainbow -T-1/1/0.1 > tmp.cpt
makegrid -R0/360/-90/90 -F
$plot -R$R1 -Y6.5i -K > $ps
$plot -R$R2 -Y-2i -O -K >> $ps
$plot -R$R3 -Y-2i -O -K >> $ps
$plot -R$R4 -Y-2i -O -K >> $ps
makegrid -R15/345/-75/75
$plot -R$R1 -Y6i -X5i -O -K >> $ps
$plot -R$R2 -Y-2i -O -K >> $ps
$plot -R$R3 -Y-2i -O -K >> $ps
$plot -R$R4 -Y-2i -O >> $ps

rm -f tmp.cpt tmp.nc .gmtcommands4

pscmp
