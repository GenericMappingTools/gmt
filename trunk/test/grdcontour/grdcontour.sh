#!/bin/bash
#
#	$Id$

. functions.sh
header "Test grdcontour for a problem on region selection"

ps=grdcontour.ps
grdcontour="grdcontour -A200 -C100 -Gd4 $src/xz-temp.nc -Jx0.4c/0.4c -Ba5f1/a5f1WNse -Gd4 -Wathin,grey -Wcdefault,grey"
$grdcontour -R-100/-60/3/21.02 -P -K > $ps
$grdcontour -R-100/-60/3/20 -O -Y10c >> $ps

pscmp
