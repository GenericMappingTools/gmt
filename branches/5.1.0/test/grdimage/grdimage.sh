#!/bin/bash
#
#	$Id$

ps=grdimage.ps

image="gmt grdimage t.nc -Ct.cpt -JX1i -B1 -BWeSn --FONT_ANNOT_PRIMARY=10p"
contour="gmt grdcontour t.nc -Ct.cpt -J -R -O"

makegrd () {
gmt grdmath $* -I1 X Y 0.2 MUL ADD = t.nc
}

label () {
gmt pstext -R -J -N -F+f10p+jBR -O -K <<%
-1.4 1.5 grdmath $1
-1.4 1.0 grdimage $2
%
}

plots () {
$image -K -R-0.5/2.5/-0.5/2.5 $1
label "$3" ""
$image -O -K -R0/2/0/2 -X4c
$image -O -K -R-0.5/1.5/0/2 -X4c
$image -O -K -R-1/3/-1/3 -X4c

$image -E50 -O -K -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c -nl ; $contour -K
label "$3" "-E50 -nl"
$image -E50 -O -K -R0/2/0/2 -X4c -nl ; $contour -K
$image -E50 -O -K -R-0.5/1.5/0/2 -X4c -nl ; $contour -K
$image -E50 -O -K -R-1/3/-1/3 -X4c -nl ; $contour -K

$image -E50 -O -K -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c ; $contour -K
label "$3" -E50
$image -E50 -O -K -R0/2/0/2 -X4c ; $contour -K
$image -E50 -O -K -R-0.5/1.5/0/2 -X4c ; $contour -K
$image -E50 -O -K -R-1/3/-1/3 -X4c ; $contour $2
}

gmt makecpt -Crainbow -T-0.1/2.5/0.2 > t.cpt
makegrd -R0/2/0/2

plots "-P -X4c -Y24c" -K "" > $ps

makegrd -R-0.5/2.5/-0.5/2.5 -r

plots "-O -X-12c -Y-4c" "" -r >> $ps

