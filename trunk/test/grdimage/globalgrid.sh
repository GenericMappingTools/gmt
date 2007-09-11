#!/bin/sh
#
#	$Id: globalgrid.sh,v 1.7 2007-09-11 22:56:12 remko Exp $

ps=globalgrid.ps
#
# Check if we can wrap global grids over longitude
#
echo -n "$0: Test grdimage for wrapping global grids:			"
plot="grdimage -Ctmp.cpt tmp.nc -JX3i/1.5i -B60f10/30f10WeSn --BASEMAP_TYPE=plain --ANNOT_FONT_SIZE_PRIMARY=10p --PLOT_DEGREE_FORMAT=DF"
#
makegrid ()
{
	awk 'BEGIN{n=12;m=0;for (j=0;j<6*n;j++) {if (m>n) m=0;print sin(m*3.1415/6);m++}}' /dev/null | \
	xyz2grd $1 -I30 -Gtmp.nc -ZTLa -fg
}

makecpt -Crainbow -T-1/1/0.1 > tmp.cpt
makegrid "-R0/360/-90/90 -F"
$plot -Rg -Y6.5i -K > $ps
$plot -Rd -Y-2i -O -K >> $ps
makegrid "-R15/345/-75/75"
$plot -Rg -Y-2i -O -K >> $ps
$plot -Rd -Y-2i -O -K >> $ps
makegrid "-R-15/345/-90/90 -F"
$plot -Rg -Y6i -X5i -O -K >> $ps
$plot -Rd -Y-2i -O -K >> $ps
makegrid "-R0/330/-75/75"
$plot -Rg -Y-2i -O -K >> $ps
$plot -Rd -Y-2i -O >> $ps

rm -f tmp.cpt tmp.nc .gmtcommands4

compare -density 100 -metric PSNR {,orig/}$ps globalgrid_diff.png > log 2>&1
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail globalgrid_diff.png log
fi
