#!/bin/sh
#
#	$Id: grdcontour.sh,v 1.8 2007-11-15 04:20:41 remko Exp $

. ../functions.sh
header "Test grdcontour for a problem on region selection"
ps=grdcontour.ps
grdcontour="grdcontour -A200 -C100 -Gd4 xz-temp.grd -Jx0.4c/0.4c -Ba5f1/a5f1WNse -Gd4 -Wa3/grey -Wc1/grey"
$grdcontour -R-100/-60/3/21.02 -P -K > $ps
$grdcontour -R-100/-60/3/20 -O -Y10c >> $ps

rm -f .gmtcommands4

pscmp
