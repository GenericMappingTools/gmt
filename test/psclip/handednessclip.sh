#!/bin/bash
# Try clipping with different handedness and -N
# draw line around working area
ps=handednessclip.ps
gmt psxy -JX3i -R0/1/0/1 -K -P -Xc << END > $ps
0 0
0 1
1 1
1 0
0 0
END
# mask inside of frame of working area
gmt psclip -J -R -O -K -N << END >> $ps
0.2 0.2
0.2 0.8
0.8 0.8
0.8 0.2
0.2 0.2
END
# fill whole area
gmt psxy -J -R -O -K -Gorange << END >> $ps
0 0
0 1
1 1
1 0
0 0
END
# clip masked inside
gmt psclip -O -K -C >> $ps
# The other way
gmt psxy -J -R -O -K -Y4i << END >> $ps
0 0
0 1
1 1
1 0
0 0
END
# mask inside of frame of working area
gmt psclip -J -R -O -K -N << END >> $ps
0.2 0.2
0.8 0.2
0.8 0.8
0.2 0.8
0.2 0.2
END
# fill whole area
gmt psxy -J -R -O -K -Gorange << END >> $ps
0 0
0 1
1 1
1 0
0 0
END
# clip masked inside
gmt psclip -O -C >> $ps
