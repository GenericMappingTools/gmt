#!/bin/bash
#	$Id: quotedclip.sh,v 1.5 2011-03-22 16:37:13 remko Exp $
#
# Check clip path and delayed text using -Sq:+e for both
# straight and curved text paths

. ../functions.sh
header "Test psxy quoted lines with clipping and delayed text"

ps=quotedclip.ps

psbasemap -R0/10/0/10 -JX15c/10c -Ggray70 -K -P -B0:."Clip path from straight text": --MAP_TITLE_OFFSET=0 --FONT_TITLE=24p > $ps

psxy -R -J -W1p,red -Sqn1:+Lh+e+s18 -O -K << EOF >> $ps
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
psxy -R -J -L -Gorange -W0.5p -O -K box.txt >> $ps

psclip -Cs -O -K >> $ps

psbasemap -R -J -Ggray70 -O -K -Y12c  -B0:."Clip path from curved text": --MAP_TITLE_OFFSET=0 --FONT_TITLE=24p >> $ps

cat << EOF > t.txt
> "In the Future, the Present will look like Today"
EOF
gmtmath -T200/335/5 -N3/0 -o1,2 T -C1 COSD -C2 SIND -Ca 5 MUL 10 ADD -C1 5 SUB = >> t.txt
psxy -R -J -W1p,red -Sqn1:+Lh+c0+s18+v+e -O -K t.txt >> $ps

psxy -R -J -L -Gorange -W0.5p -O -K box.txt >> $ps

rm -f box.txt t.txt

psclip -Cc -O >> $ps

pscmp
