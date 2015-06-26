#!/bin/bash
# $Id$
# Testing that map scales with various labels have reasonable panels behind them
ps=mapscales.ps
gmt psbasemap -R-10/10/-15/15 -JM15c -Bafg90 -BWSne+gazure1 -Lg0/14+c14+f+w1000k+l+jr -F+gcornsilk1+p0.5p,black -P -K -Xc > $ps
gmt psbasemap -R -J -Lg0/11+c11+w1000k+f+l+jl -F+gcornsilk1+p0.5p,black -O -K >> $ps
gmt psbasemap -R -J -Lg0/8+c8+w1000k+f+l+jt -F+gcornsilk1+p0.5p,black -O -K >> $ps
gmt psbasemap -R -J -Lg0/4+c4+w1.5e6e+f+l"My own map label"+jt+u -F+gcornsilk1+p0.5p,black -O -K >> $ps
gmt psbasemap -R -J -Lg4/0+c0+w400n+f+l+jl -F+gcornsilk1+p0.5p,black+s -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> $ps
gmt psbasemap -R -J -Lg0/-4+c-4+w500M+f+l+jl -F+gcornsilk1+p0.5p,black+i+r -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> $ps
gmt psbasemap -R -J -L0/-7+c-7+w500M -F+gcornsilk1+p0.5p,black+s -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> $ps
gmt psbasemap -R -J -L0/-10+c-10+w500n+u -F+gcornsilk1+p0.5p,black -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> $ps
gmt psbasemap -R -J -L0/-12.5+c-12.5+w3e6f -F+gcornsilk1+p0.5p,black -O -K --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p --FORMAT_FLOAT_MAP=%\'.10g >> $ps
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
