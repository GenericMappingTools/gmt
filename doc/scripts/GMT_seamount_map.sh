#!/usr/bin/env bash
# Illustrate the circular and elliptical bases of seamounts
gmt begin GMT_seamount_map
    gmt set GMT_THEME cookbook
    gmt set MAP_VECTOR_SHAPE 0.5

# Circle
    echo 0 0 | gmt plot -R-1/1/-1/1 -Jx1i -Sc1.3i -W2p
    echo 0 0 | gmt plot -Sc0.1i -Gblack
    gmt plot -Sv0.1i+e+s -Gblack -W0.5p -N << EOF
-0.8	0	1	0
0	-0.8	0	1
EOF
    gmt plot -W0.25p,- << EOF
0	0
0.45	0.45
EOF
    echo 0.55 0.55 r@-0@- | gmt text -F+f14p,Times-Italic
    echo "0 0 lon,lat "| gmt text -F+f14p,Times-Italic+jTR -Dj0.05i
    echo circular | gmt text -F+f16p+cTL
    # Ellipse
    echo 0 0 30 1.7i 0.7i | gmt plot -Se -W2p -X2.5i
    echo 0 0 | gmt plot -Sc0.1i -Gblack
    gmt plot -Sv0.1i+e+s -Gblack -W0.5p -N << EOF
-0.8	0	1	0
0	-0.8	0	1
EOF
    gmt plot -W0.25p,- << EOF
>
0	0
0.736121593217	0.425
>
0	0
-0.175	0.303108891325
EOF
    echo 0 0 0.2i 30 90 | gmt plot -Sm4p+b -Gblack -W0.25p
    echo 0.736121593217 0.425 major | gmt text -F+f14p,Times-Italic+jBL -Dj0.03i -N
    echo -0.175	0.303108891325 minor | gmt text -F+f14p,Times-Italic+jBR -Dj0.03i -N
    echo 0.125 0.275 @~a@~ | gmt text -F+f14p,Times-Italic
    echo "0 0 lon,lat "| gmt text -F+f14p,Times-Italic+jTR -Dj0.05i
    echo elliptical | gmt text -F+f16p+cTL
    gmt plot -T
gmt end show
