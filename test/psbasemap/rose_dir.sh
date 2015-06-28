#!/bin/bash
#	$Id$
# Testing map directional roses
ps=rose_dir.ps
gmt set FONT_LABEL 10p FONT_TITLE 14p MAP_ANNOT_OBLIQUE 34 MAP_TITLE_OFFSET 7p MAP_FRAME_WIDTH 3p
# 4th row left: Fancy kind = 1
gmt psbasemap -R-7/7/-5/5 -JM2.75i -Baf -BWSne+gazure1 -Tdg0/0+w1i+f1 -P -K -X1.25i > $ps
# 4th row right: Fancy kind = 2
gmt psbasemap -R -J -Baf -BwSnE+gazure1 -Tdg0/0+w1i+f2 -O -K -X3.25i >> $ps
# 3rd row left: Fancy kind = 3
gmt psbasemap -R -J -Baf -BWsne+gazure1 -Tdg0/0+w1i+f3 -O -K -X-3.25i -Y2.2i >> $ps
# 3rd row right: Plain kind
gmt psbasemap -R -J -Baf -BwsnE+gazure1 -Tdg0/0+w1i -O -K -X3.25i >> $ps
# 2nd row left: Fancy kind = 1
gmt psbasemap -R-10/-2.5/10/2.5r -JOc0/0/50/60/2.75i -Baf -BWSne+gazure1 -Tdg0/0+w1i+f1 -O -K -X-3.25i -Y2.4i >> $ps
# 2nd row right: Fancy kind = 2
gmt psbasemap -R -J -Baf -BwSnE+gazure1 -Tdg0/0+w1i+f2 -O -K -X3.25i >> $ps
# 1st row left: Fancy kind = 3
gmt psbasemap -R -J -Baf -BWsne+gazure1 -Tdg0/0+w1i+f3 -O -K -X-3.25i -Y2.4i >> $ps
# 1st row right: Plain kind
gmt psbasemap -R -J -Baf -BwsnE+gazure1 -Tdg0/0+w1i -O -X3.25i >> $ps
