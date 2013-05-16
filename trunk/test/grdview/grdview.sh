#!/bin/bash
#
#	$Id$

ps=grdview.ps

view="gmt grdview t.nc -Ct.cpt -JX1i -B1 -BWeSn --FONT_ANNOT_PRIMARY=10p"
contour="gmt grdcontour t.nc -Ct.cpt -J -R -O"

makegrd () {
gmt grdmath $* -I1 X Y 0.2 MUL ADD = t.nc
}

label () {
gmt pstext -R -J -F+f10p+jBR -N -O -K <<%
-1.4 1.5 grdmath $2
-1.4 1.0 grdview $1
%
}

plots () {
$view -K -R-0.5/2.5/-0.5/2.5 $1 -Qs
$contour -K
label -Qs $3
$view -O -K -R0/2/0/2 -X4c -Qs
$contour -K
$view -O -K -R-0.5/1.5/0/2 -X4c -Qs
$contour -K
$view -O -K -R-1/3/-1/3 -X4c -Qs
$contour -K
$view -O -K -Towhite -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c
label -T $3
$view -O -K -Towhite -R0/2/0/2 -X4c
$view -O -K -Towhite -R-0.5/1.5/0/2 -X4c
$view -O -K -Towhite -R-1/3/-1/3 -X4c
$view -Qi100 -O -K -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c
$contour -K
label -Qi100 $3
$view -Qi100 -O -K -R0/2/0/2 -X4c
$contour -K
$view -Qi100 -O -K -R-0.5/1.5/0/2 -X4c
$contour -K
$view -Qi100 -O -K -R-1/3/-1/3 -X4c
$contour $2
}

gmt makecpt -Crainbow -T-0.1/2.5/0.2 > t.cpt
makegrd -R0/2/0/2

plots "-P -X4c -Y24c" -K " " > $ps

makegrd -R-0.5/2.5/-0.5/2.5 -r

plots "-O -X-12c -Y-4c" " " -r >> $ps
