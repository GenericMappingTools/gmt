#!/bin/bash
#
#	$Id: grdimage.sh,v 1.13 2011-04-25 00:21:07 guru Exp $

. ../functions.sh
header "Test grdimage for grid and pixel plots"

ps=grdimage.ps
grdimage="grdimage t.nc -Ct.cpt -JX1i -B1/1WeSn --FONT_ANNOT_PRIMARY=10p"
grdcontour="grdcontour t.nc -Ct.cpt -J -R -O"

makegrd () {
grdmath $* -I1 X Y 0.2 MUL ADD = t.nc
}

label () {
pstext -R -J -N -F+f10p+jBR -O -K <<%
-1.4 1.5 xyz2grd $1
-1.4 1.0 grdimage $2
%
}

plots () {
$grdimage -K -R-0.5/2.5/-0.5/2.5 $1
label "$3" ""
$grdimage -O -K -R0/2/0/2 -X4c
$grdimage -O -K -R-0.5/1.5/0/2 -X4c
$grdimage -O -K -R-1/3/-1/3 -X4c

$grdimage -E50 -O -K -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c -nl ; $grdcontour -K
label "$3" "-E50 -nl"
$grdimage -E50 -O -K -R0/2/0/2 -X4c ; $grdcontour -K -nl
$grdimage -E50 -O -K -R-0.5/1.5/0/2 -X4c ; $grdcontour -K -nl
$grdimage -E50 -O -K -R-1/3/-1/3 -X4c ; $grdcontour -K -nl

$grdimage -E50 -O -K -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c ; $grdcontour -K
label "$3" -E50
$grdimage -E50 -O -K -R0/2/0/2 -X4c ; $grdcontour -K
$grdimage -E50 -O -K -R-0.5/1.5/0/2 -X4c ; $grdcontour -K
$grdimage -E50 -O -K -R-1/3/-1/3 -X4c ; $grdcontour $2
}

makecpt -Crainbow -T-0.1/2.5/0.2 > t.cpt
makegrd -R0/2/0/2

plots "-P -X4c -Y24c" -K "" > $ps

makegrd -R-0.5/2.5/-0.5/2.5 -r

plots "-O -X-12c -Y-4c" "" -r >> $ps

rm -f t.nc t.cpt

pscmp
