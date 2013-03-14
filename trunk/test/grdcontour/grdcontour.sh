#!/bin/bash
#
#	$Id$

ps=grdcontour.ps

grdcontour="grdcontour -A200 -C100 -Gd4 xz-temp.nc -Jx0.4c/0.4c -Ba5f1/a5f1WNse -Wathin,grey -Wcdefault,grey"
$grdcontour -R-100/-60/3/21.02 -P -K -Y1.5c > $ps
$grdcontour -R-100/-60/3/20 -O -K -Y9c >> $ps
echo 100 C >  cont.dat
echo 200 A >> cont.dat
echo 300 C >> cont.dat
echo 400 A >> cont.dat
echo 500 C >> cont.dat
echo 600 A >> cont.dat
echo 700 C >> cont.dat
echo 800 A >> cont.dat
echo 900 C >> cont.dat
grdcontour -Ccont.dat xz-temp.nc -Jx -R-100/-60/3/20 -Ba5f1/a5f1WNse -Gd4 -Wathin,grey -Wcdefault,grey -O -Y9c >> $ps
