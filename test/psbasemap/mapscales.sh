#!/bin/bash
# $Id$
# Testing that map scales with various labels have reasonable panels behind them
ps=mapscales.ps
rm -f gmt.conf
gmt psbasemap -R-10/10/-15/15 -JM15c -Bafg90 -BWSne+gazure1 -Lf0/14/14/1000k+l+jr -F+gcornsilk1+p0.5p,black -P -K -Xc > $ps
gmt psbasemap -R -J -Lf0/11/11/1000k+l+jl -F+gcornsilk1+p0.5p,black -O -K >> $ps
gmt psbasemap -R -J -Lf0/8/8/1000k+l+jt -F+gcornsilk1+p0.5p,black -O -K >> $ps
gmt psbasemap -R -J -Lf0/4/4/1.5e6e+l"My own map label"+jt+u -F+gcornsilk1+p0.5p,black -O -K >> $ps
gmt psbasemap -R -J -Lf4/0/0/400n+l+jl -F+gcornsilk1+p0.5p,black+s -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> $ps
gmt psbasemap -R -J -Lf0/-4/-4/500M+l+jl -F+gcornsilk1+p0.5p,black+i+r -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> $ps
gmt psbasemap -R -J -L0/-7/-7/500M -F+gcornsilk1+p0.5p,black+s -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> $ps
gmt psbasemap -R -J -L0/-10/-10/500n+u -F+gcornsilk1+p0.5p,black -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> $ps
gmt psbasemap -R -J -L0/-12.5/-12.5/3e6f -F+gcornsilk1+p0.5p,black -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p --FORMAT_FLOAT_MAP=%\'.10g >> $ps
# Plot a red cross at the justification point for the scales
gmt psxy -R -J -O -Sx0.2i -W0.5p,red << EOF >> $ps
0	14
0	11
0	8
0	4
4	0
0	-4
0	-7
0	-10
0	-12.5
EOF
