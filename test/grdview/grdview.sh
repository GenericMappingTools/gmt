#!/bin/bash
#
#	$Id$

. ./functions.sh
header "Test grdview for grid and pixel plots"

grdview=grdview" t.nc -Ct.cpt -JX1i -B1/1WeSn --FONT_ANNOT_PRIMARY=10p"
grdcontour=grdcontour" t.nc -Ct.cpt -J -R -O"

makegrd () {
grdmath $* -I1 X Y 0.2 MUL ADD = t.nc
}

label () {
pstext -R -J -F+f10p+jBR -N -O -K <<%
-1.4 1.5 grdmath $2
-1.4 1.0 grdview $1
%
}

plots () {
$grdview -K -R-0.5/2.5/-0.5/2.5 $1 -Qs
$grdcontour -K
label -Qs $3
$grdview -O -K -R0/2/0/2 -X4c -Qs
$grdcontour -K
$grdview -O -K -R-0.5/1.5/0/2 -X4c -Qs
$grdcontour -K
$grdview -O -K -R-1/3/-1/3 -X4c -Qs
$grdcontour -K
$grdview -O -K -Towhite -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c
label -T $3
$grdview -O -K -Towhite -R0/2/0/2 -X4c
$grdview -O -K -Towhite -R-0.5/1.5/0/2 -X4c
$grdview -O -K -Towhite -R-1/3/-1/3 -X4c
$grdview -Qi100 -O -K -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c
$grdcontour -K
label -Qi100 $3
$grdview -Qi100 -O -K -R0/2/0/2 -X4c
$grdcontour -K
$grdview -Qi100 -O -K -R-0.5/1.5/0/2 -X4c
$grdcontour -K
$grdview -Qi100 -O -K -R-1/3/-1/3 -X4c
$grdcontour $2
}

makecpt -Crainbow -T-0.1/2.5/0.2 > t.cpt
makegrd -R0/2/0/2

plots "-P -X4c -Y24c" -K " " > $ps

makegrd -R-0.5/2.5/-0.5/2.5 -r

plots "-O -X-12c -Y-4c" " " -r >> $ps

pscmp
