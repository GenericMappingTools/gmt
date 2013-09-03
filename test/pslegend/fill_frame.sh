#!/bin/bash
#	$Id$
#
# Testing gmt pslegend fill and frames

ps=fill_frame.ps
color=yellow

gmt psbasemap -R0/15/0/25 -Jx1c -P -B0 -K -Y1c > $ps
gmt pslegend -R -J -O -K -Dx1c/24c/8c/1c/TL -F+g$color >> $ps <<< "L - - C $color fill, no frame"
gmt pslegend -R -J -O -K -Dx1c/22c/8c/1c/TL -F+p+g$color >> $ps <<< "L - - C $color fill, black frame"
gmt pslegend -R -J -O -K -Dx1c/20c/8c/1c/TL -F+p2p,blue+g${color}@50 >> $ps <<< "L - - C $color@@50 fill, blue frame"
gmt pslegend -R -J -O -K -Dx1c/18c/8c/1c/TL -F+p2p,blue@50+g$color >> $ps <<< "L - - C $color fill, blue@@50 frame"
gmt pslegend -R -J -O -K -Dx1c/15c/8c/TL -F+p+g$color >> $ps <<%
P
T This text wraps to the next line. It uses all standard paragraph modes. But what if you want to use left alignment?
%
gmt pslegend -R -J -O -K -Dx1c/12c/8c/TL -F+p+g$color >> $ps <<%
P - - - - - - - l
T Same trying to do left alignment. That seems to work nicely but needs 8 parameters on P line.
%
gmt psxy -R -J -O -T >> $ps
