#!/usr/bin/env bash
#
# Highlight the problem of line obliquely exiting and being cut in an ugly way
ps=trimline.ps

gmt psxy -R0/6/0/6 -Jx1i --MAP_FRAME_TYPE=plain -W40p -Baf -BWSne -P -X1.25i << EOF > $ps
-1 3
3 3
7 5
EOF

# The orig was faked using this script:
#gmt psclip -R0/6/0/6 -Jx1i -T -K -P -X1.25i > $ps
#gmt psxy -R-0.1/6.1/0/6 -J --MAP_FRAME_TYPE=plain -W40p -X-0.1i -O -K << EOF >> $ps
#-1 3
#3 3
#7 5
#EOF
#gmt psclip -R0/6/0/6 -J -O -K -C >> $ps
#gmt psbasemap -R -Jx1i --MAP_FRAME_TYPE=plain -Baf -BWSne -X0.1i -O >> $ps
