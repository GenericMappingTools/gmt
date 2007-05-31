#!/bin/sh
#
#	$Id: grdcontour.sh,v 1.5 2007-05-31 02:51:31 pwessel Exp $

echo -n "$0: Test grdcontour for a problem on region selection:	"
ps=grdcontour.ps
grdcontour="grdcontour -A200 -C100 -Gd4 xz-temp.grd -Jx0.4/0.4 -Ba5f1/a5f1WNse -Gd4 -Wa3/grey -Wc1/grey"
$grdcontour -R-100/-60/3/21.02 -K > $ps
$grdcontour -R-100/-60/3/20 -O -Y10c >> $ps

rm -f .gmtcommands4

compare -density 100 -metric PSNR grdcontour_orig.ps $ps grdcontour_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail grdcontour_diff.png log
fi
