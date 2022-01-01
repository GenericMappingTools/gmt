#!/usr/bin/env bash
# Test gmt pstext with composite characters.
ps=composite.ps
gmt set PS_CHAR_ENCODING ISOLatin1+
cat << EOF > tmp
8	2	Writing @!\227\145 in Times-Roman
8	6	Writing @!\227@~\145@~ in Symbol
8	10	Writing @!\250@%Times-Italic%s@%% in Times-Italic
8	14	University of Hawaii at M@!a\225noa
EOF
gmt pstext tmp -R0/16/0/16 -Jx1c -B0 -P -F+f32p,Times-Roman+jCM > $ps
