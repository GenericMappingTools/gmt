#!/bin/sh
#
#	$Id: grdimage.sh,v 1.10 2010-06-21 23:55:22 guru Exp $

ps=grdimage.ps
grdimage=grdimage" $VERBOSE t.nc -Ct.cpt -JX1i -B1/1WeSn --ANNOT_FONT_SIZE=10p"
grdcontour=grdcontour" t.nc -Ct.cpt -J -R -O"

. ../functions.sh
header "Test grdimage for grid and pixel plots"
makegrd () {
xyz2grd -I1 -Gt.nc $* <<%
0 0 0.0
0 1 0.2
0 2 0.4
1 0 1.0
1 1 1.2
1 2 1.4
2 0 2.0
2 1 2.2
2 2 2.4
%
}

label () {
pstext -R -J -N -O -K <<%
-1.4 1.5 10 0 0 BR xyz2grd $1
-1.4 1 10 0 0 BR grdimage $2
%
}

plots () {
$grdimage -K -R-0.5/2.5/-0.5/2.5 $1
label "$3" ""
$grdimage -O -K -R0/2/0/2 -X4c
$grdimage -O -K -R-0.5/1.5/0/2 -X4c
$grdimage -O -K -R-1/3/-1/3 -X4c

$grdimage -E50 -Sl -O -K -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c ; $grdcontour -K
label "$3" "-E50 -Sl"
$grdimage -E50 -Sl -O -K -R0/2/0/2 -X4c ; $grdcontour -K
$grdimage -E50 -Sl -O -K -R-0.5/1.5/0/2 -X4c ; $grdcontour -K
$grdimage -E50 -Sl -O -K -R-1/3/-1/3 -X4c ; $grdcontour -K

$grdimage -E50 -O -K -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c ; $grdcontour -K
label "$3" -E50
$grdimage -E50 -O -K -R0/2/0/2 -X4c ; $grdcontour -K
$grdimage -E50 -O -K -R-0.5/1.5/0/2 -X4c ; $grdcontour -K
$grdimage -E50 -O -K -R-1/3/-1/3 -X4c ; $grdcontour $2
}

makecpt -Crainbow -T-0.1/2.5/0.2 > t.cpt
makegrd -R0/2/0/2

plots "-P -X4c -Y24c" -K " " > $ps

makegrd -R-0.5/2.5/-0.5/2.5 -F

plots "-O -X-12c -Y-4c" " " -F >> $ps

rm -f t.nc t.cpt .gmtcommands4

pscmp
