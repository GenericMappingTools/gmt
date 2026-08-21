#!/usr/bin/env bash
#
# Check custom symbol needed 2 column strings from trailing text
# and using size to scale the texts.  Note words must be separated
# by a single tab or space

ps=custom_textsymbol.ps

cat << EOF > t.txt
# LON  LAT  SIZE  STRING1  STRING2
# --------------------------------
118	0	1c	one ONE
119	1	2c	two	TWO
120	2	3c	three THREE
EOF

cat << EOF > text_subst_symbol.def
N: 2 ss
0 0 1 \$t0 l -Gred
0 0 1 \$t1 l -Gblue
EOF
gmt pscoast -R117/-1.5/122.5/3r -JM15c -Bag -Di -Ggrey -Wthinnest -A250 -P -K -Xc > $ps
gmt psxy -R -J t.txt -Sktext_subst_symbol -O >> $ps
