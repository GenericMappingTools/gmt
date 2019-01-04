#!/usr/bin/env bash
#
# Check clip path and delayed text using -Sq:+e for both
# straight and curved text paths

ps=quotedclip.ps

gmt psbasemap -R0/10/0/10 -JX15c/10c -K -P -B+t"Clip path from straight text"+ggray70 --MAP_TITLE_OFFSET=0 --FONT_TITLE=24p > $ps

gmt psxy -R -J -W1p,red -Sqn1:+Lh+e+f18p -O -K << EOF >> $ps
> "The quick brown fox jumps over the lazy dog"
0 8
10 5
EOF

# small polygon (should not cover up text path)
cat << EOF > box.txt
7 4
7 8
9 8
9 4
EOF
gmt psxy -R -J -L -Gorange -W0.5p -O -K box.txt >> $ps

gmt psclip -C -O -K >> $ps

gmt psbasemap -R -J -O -K -Y12c -B+t"Clip path from curved text"+ggray70 --MAP_TITLE_OFFSET=0 --FONT_TITLE=24p >> $ps

cat << EOF > t.txt
> "In the Future, the Present will look like Today"
EOF
gmt math -T200/335/5 -N3/0 -o1,2 T -C1 COSD -C2 SIND -Ca 5 MUL 10 ADD -C1 5 SUB = >> t.txt
gmt psxy -R -J -W1p,red -Sqn1:+Lh+c0+f18p+v+e -O -K t.txt >> $ps

gmt psxy -R -J -L -Gorange -W0.5p -O -K box.txt >> $ps

gmt psclip -C -O >> $ps

