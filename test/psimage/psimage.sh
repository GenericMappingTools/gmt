#!/usr/bin/env bash
#

ps=psimage.ps

cat > tt.in <<%
0 0
1 0 +fwhite+bblack
0 1 +fblack+b-
1 1 +f-+bblack
0 2 +fwhite+b-
1 2 +f-+bwhite
0 3 +fred+b-
1 3 +f-+bred
0 4 +fred+byellow
1 4 +fyellow+bred
%
gmt psxy -R0/3/0/5 -Jx1.5i -Gp@circuit.png+r128 -P -K > $ps <<%
0 0
2 0
3 1
3 4
2 5
0 5
%
$AWK '{ x0=$1;x1=x0+1;y0=$2;y1=y0+1;c=$3; \
	printf "> -Gp10+r80%s\n%i %i\n%i %i\n%i %i\n",c,x0,y0,x1,y1,x0,y1 ; \
	printf "> -GP10+r80%s\n%i %i\n%i %i\n%i %i\n",c,x0,y0,x1,y1,x1,y0}' < tt.in \
	| gmt psxy -R -J -O -K >> $ps
gmt psxy -R -J -O -K <<% >> $ps
> -Gyellow
2 4
2.5 4
2.5 4.5
> -Gred -W2p,blue
2 4
2.5 4.5
2 4.5
%
$AWK '{ x0=$1+0.5;y0=$2+0.5;c=$3; \
	printf "%g %g BR p%s\n",x0,y0,c ; \
	printf "%g %g TL P%s\n",x0,y0,c}' < tt.in \
	| gmt pstext -F+f7p,Helvetica-Bold,purple+j -R -J -O -K >> $ps
echo 2.5 2.5 | gmt psxy -R -J -O -K -Ss1i -Gp10+fred+b-+r80 >> $ps
echo 2.5 1.5 | gmt psxy -R -J -O -Ss1i -Gp10+r80 >> $ps
