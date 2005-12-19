#!/bin/sh

ps=position1.ps

psbasemap --ANNOT_FONT_SIZE=24p -JX8i/5i -R0/12/0/12 -B1g1/1SW -K > $ps
pstext -R -J -O -K >> $ps <<%
1 1 24 0 0 BR 1
1 2 24 0 0 BC 1
1 3 24 0 0 BL 1
2 1 24 0 0 BR 2
2 2 24 0 0 BC 2
2 3 24 0 0 BL 2
3 1 24 0 0 BR 3
3 2 24 0 0 BC 3
3 3 24 0 0 BL 3
4 1 24 0 0 BR BR
4 2 24 0 0 BC BC
4 3 24 0 0 BL BL
6 1 24 0 0 BR 1A
6 2 24 0 0 BC 1A
6 3 24 0 0 BL 1A
7 1 24 0 0 BR 2A
7 2 24 0 0 BC 2A
7 3 24 0 0 BL 2A
8 1 24 0 0 BR Z1
8 2 24 0 0 BC Z1
8 3 24 0 0 BL Z1
9 1 24 0 0 BR 9
9 2 24 0 0 BC 9
9 3 24 0 0 BL 9
10 1 24 0 0 BR 10
10 2 24 0 0 BC 10
10 3 24 0 0 BL 10
11 1 24 0 0 BR 11
11 2 24 0 0 BC 11
11 3 24 0 0 BL 11
%
psbasemap --ANNOT_FONT_SIZE=24p -J -R0/1.2/0/1.2 -B0.1/0.1NE -O >> $ps

rm -f .gmtcommands4

echo -n "Comparing position1_orig.ps and position1.ps: "
compare -density 100 -metric PSNR position1_orig.ps position1.ps position1_diff.png
