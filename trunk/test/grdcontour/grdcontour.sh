#!/bin/sh
ps=grdcontour.ps
grdcontour="grdcontour -A200 -C100 -Gd4 xz-temp.grd -Jx0.4/0.4 -Ba5f1/a5f1WNse -Gd4 -Wa3/grey -Wc1/grey"
$grdcontour -R-100/-60/3/21.02 -K > $ps
$grdcontour -R-100/-60/3/20 -O -Y10c >> $ps

rm -f .gmtcommands4

echo -n "GMT: Test grdcontour for a problem on region selection:          "
compare -density 100 -metric PSNR grdcontour_orig.ps $ps grdcontour_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAILED]"
else
        echo "[OK"]
        rm -f fail grdcontour_diff.png log
fi
