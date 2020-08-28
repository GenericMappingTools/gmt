#!/usr/bin/env bash
# Illustrate the circular and elliptical bases of seamounts
ps=GMT_seamount_map.ps
gmt set MAP_VECTOR_SHAPE 0.5

# Circle
echo 0 0 | gmt psxy -R-1/1/-1/1 -Jx1i -Sc1.3i -W2p -P -K > $ps
echo 0 0 | gmt psxy -R -J -Sc0.1i -Gblack -O -K >> $ps
gmt psxy -R -J -O -K -Sv0.1i+e+s -Gblack -W0.5p -N << EOF >> $ps
-0.8	0	1	0
0	-0.8	0	1
EOF
gmt psxy -R -J -O -K -W0.25p,- << EOF >> $ps
0	0
0.45	0.45
EOF
echo 0.55 0.55 r@-0@- | gmt pstext -R -J -O -K -F+f14p,Times-Italic >> $ps
echo "0 0 lon,lat "| gmt pstext -R -J -O -K -F+f14p,Times-Italic+jTR -Dj0.05i >> $ps
echo circular | gmt pstext -R -J -O -K -F+f16p+cTL >> $ps
# Ellipse
echo 0 0 30 1.7i 0.7i | gmt psxy -R -J -O -K -Se -W2p -X2.5i >> $ps
echo 0 0 | gmt psxy -R -J -Sc0.1i -Gblack -O -K >> $ps
gmt psxy -R -J -O -K -Sv0.1i+e+s -Gblack -W0.5p -N << EOF >> $ps
-0.8	0	1	0
0	-0.8	0	1
EOF
gmt psxy -R -J -O -K -W0.25p,- << EOF >> $ps
>
0	0
0.736121593217	0.425
>
0	0
-0.175	0.303108891325
EOF
echo 0 0 0.2i 30 90 | gmt psxy -R -J -O -K -Sm4p+b -Gblack -W0.25p >> $ps
echo 0.736121593217 0.425 major | gmt pstext -R -J -O -K -F+f14p,Times-Italic+jBL -Dj0.03i -N >> $ps
echo -0.175	0.303108891325 minor | gmt pstext -R -J -O -K -F+f14p,Times-Italic+jBR -Dj0.03i -N >> $ps
echo 0.125 0.275 @~a@~ | gmt pstext -R -J -O -K -F+f14p,Times-Italic >> $ps
echo "0 0 lon,lat "| gmt pstext -R -J -O -K -F+f14p,Times-Italic+jTR -Dj0.05i >> $ps
echo elliptical | gmt pstext -R -J -O -K -F+f16p+cTL >> $ps
gmt psxy -R -J -O -T >> $ps
