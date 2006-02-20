#!/bin/sh
ps=grdimage.ps
grdimage=grdimage" t.grd -Ct.cpt -JX1i -B1/1"
grdcontour=grdcontour" t.grd -Ct.cpt -J -R -O"

makegrd () {
xyz2grd -I1 -Gt.grd $* <<%
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
label $3 ""
$grdimage -O -K -R0/2/0/2 -X4c
$grdimage -O -K -R-0.5/1.5/0/2 -X4c
$grdimage -O -K -R-1/3/-1/3 -X4c
$grdimage -O -K -Towhite -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c
label $3 -T
$grdimage -O -K -Towhite -R0/2/0/2 -X4c
$grdimage -O -K -Towhite -R-0.5/1.5/0/2 -X4c
$grdimage -O -K -Towhite -R-1/3/-1/3 -X4c
$grdimage -E50 -O -K -R-0.5/2.5/-0.5/2.5 -X-12c -Y-4c
$grdcontour -K
label $3 -E50
$grdimage -E50 -O -K -R0/2/0/2 -X4c
$grdcontour -K
$grdimage -E50 -O -K -R-0.5/1.5/0/2 -X4c
$grdcontour -K
$grdimage -E50 -O -K -R-1/3/-1/3 -X4c
$grdcontour $2
}

makecpt -Crainbow -T-0.1/2.5/0.2 > t.cpt
makegrd -R0/2/0/2

plots "-P -X4c -Y24c" -K " " > $ps

makegrd -R-0.5/2.5/-0.5/2.5 -F

plots "-O -X-12c -Y-4c" " " -F >> $ps

rm -f t.grd t.cpt .gmtcommands4

echo -n "Comparing grdimage_orig.ps and $ps: "
compare -density 100 -metric PSNR grdimage_orig.ps $ps grdimage_diff.png
