#!/bin/sh
#
#	$Id: grdcontour.sh,v 1.6 2007-06-05 14:02:35 remko Exp $

echo -n "$0: Test grdcontour for a problem on region selection:	"
ps=grdcontour.ps
grdcontour="grdcontour -A200 -C100 -Gd4 xz-temp.grd -Jx0.4c/0.4c -Ba5f1/a5f1WNse -Gd4 -Wa3/grey -Wc1/grey"
$grdcontour -R-100/-60/3/21.02 -P -K > $ps
$grdcontour -R-100/-60/3/20 -O -Y10c >> $ps

rm -f .gmtcommands4

compare -density 100 -metric PSNR {,orig/}$ps grdcontour_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail grdcontour_diff.png log
fi
