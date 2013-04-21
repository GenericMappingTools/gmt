#!/bin/bash
#	$Id: fancy.sh 11026 2013-03-01 01:06:56Z pwessel $
#
# Testing pslegend fill and frames

ps=fill_frame.ps
color=yellow

psbasemap -R0/15/0/25 -Jx1c -P -B0 -K -Y1c > $ps
pslegend -R -J -O -K -Dx1c/24c/8c/1c/TL -F+f$color >> $ps <<< "L - - C $color fill, no frame"
pslegend -R -J -O -K -Dx1c/22c/8c/1c/TL -F+p+f$color >> $ps <<< "L - - C $color fill, black frame"
pslegend -R -J -O -K -Dx1c/20c/8c/1c/TL -F+p2p,blue+f${color}@50 >> $ps <<< "L - - C $color@@50 fill, blue frame"
pslegend -R -J -O -K -Dx1c/18c/8c/1c/TL -F+p2p,blue@50+f$color >> $ps <<< "L - - C $color fill, blue@@50 frame"
pslegend -R -J -O -K -Dx1c/15c/8c/TL -F+p+f$color >> $ps <<%
P
T This text wraps to the next line. It uses all standard paragraph modes. But what if you want to use left alignment?
%
pslegend -R -J -O -K -Dx1c/12c/8c/TL -F+p+f$color >> $ps <<%
P - - - - - - - l
T Same trying to do left alignment. That seems to work nicely but needs 8 parameters on P line.
%
psxy -R -J -O -T >> $ps
