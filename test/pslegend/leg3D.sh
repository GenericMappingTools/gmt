#!/bin/bash
#	$Id$
#
# Testing gmt pslegend in 3D [http://gmt.soest.hawaii.edu/boards/1/topics/5157]

ps=leg3D.ps
gmt pslegend -R-85/-33/-58/15 -JM15c -p210/40 -P leg.txt -DjLB+o0.2i+w2.2i/0+jBL -F+glightgrey+pthinner+s-4p/-6p/grey20@40 > $ps
