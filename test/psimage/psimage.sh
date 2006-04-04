#!/bin/sh
ps=psimage.ps
psimage -E128 -N2/2 water.ras -K > $ps
cat > t.in <<%
0 0
0 1 :FwhiteBblack
1 0 :FblackB-
1 1 :F-Bblack
2 0 :FwhiteB-
2 1 :F-Bwhite
3 0 :FredB-
3 1 :F-Bred
4 0 :FredByellow
4 1 :FyellowBred
%
awk '{ x0=$1;x1=x0+1;y0=$2;y1=y0+1;c=$3; \
	printf "> -Gp80/10%s\n%i %i\n%i %i\n%i %i\n",c,x0,y0,x1,y1,x0,y1 ; \
	printf "> -GP80/10%s\n%i %i\n%i %i\n%i %i\n",c,x0,y0,x1,y1,x1,y0}' < t.in \
	| psxy -R0/6/0/3 -JX8i/4i -M -O -K >> $ps
awk '{ x0=$1+0.5;y0=$2+0.5;c=$3; \
	printf "%g %g 7 0 1 BR p%s\n",x0,y0,c ; \
	printf "%g %g 7 0 1 TL P%s\n",x0,y0,c}' < t.in \
	| pstext -Ggreen -R -J -O -K >> $ps
psimage -E80 -C3i/3i/BL $GMTHOME/share/pattern/ps_pattern_10.ras -Gfred -Gb- -O -K >> $ps
psimage -E80 -C3i/3i/BR $GMTHOME/share/pattern/ps_pattern_10.ras -O >> $ps
rm -f t.in .gmtcommands4

echo -n "Comparing psimage_orig.ps and $ps: "
compare -density 100 -metric PSNR psimage_orig.ps $ps psimage_diff.png
