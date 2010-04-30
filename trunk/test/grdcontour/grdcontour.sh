#!/bin/sh
#
#	$Id: grdcontour.sh,v 1.9 2010-04-30 02:50:08 guru Exp $

. ../functions.sh
header "Test grdcontour for a problem on region selection"
ps=grdcontour.ps
grdcontour="grdcontour -A200 -C100 -Gd4 xz-temp.grd -Jx0.4c/0.4c -Ba5f1/a5f1WNse -Gd4 -Wa3,grey -Wc1,grey"
$grdcontour -R-100/-60/3/21.02 -P -K > $ps
$grdcontour -R-100/-60/3/20 -O -Y10c >> $ps

rm -f .gmtcommands4

pscmp
