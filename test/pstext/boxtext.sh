#!/bin/bash
#	$Id$
# Compare the size and alignment of identical text plotted in different ways:
# (lower): Plain text with box and margin
# (upper): Plain text with textbox clip path, then red paint
ps=boxtext.ps
gmt set PS_COMMENTS true
gmt pstext -R0/6/0/4 -Jx1i -Ba1 -P -Dj0.5i/0.5i -F+f32p+jLB -Gyellow -W0.25p,green -TO -K -C1c << EOF > $ps
2	2	TEXT
EOF
gmt pstext -R -J -O -K -Dj0.5i/0.5i -F+f32p+jLB -Gc -C1c -TO -Y5i << EOF >> $ps
2	2	TEXT
EOF
gmt psxy -R -J -O -K -Sri -Gred << EOF >> $ps
3	2	6	4
EOF
gmt psclip -R -J -C -O -K >> $ps
# Plot alignment lines to hightlight misalignment
gmt psxy -R0/6/0/9 -Jx1i -O -Y-5i -W0.5p,blue << EOF >> $ps
>
2.115 0
2.115 9
>
4.025 0
4.025 9
>
0	2.105
6	2.105
>
0	3.22
6	3.22
>
0	7.105
6	7.105
>
0	8.22
6	8.22
EOF
