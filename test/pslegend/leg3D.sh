#!/usr/bin/env bash
#
# Testing gmt pslegend in 3D [http://gmt.soest.hawaii.edu/boards/1/topics/5157]

ps=leg3D.ps
gmt pslegend -R-85/-33/-58/15 -JM15c -p210/40 -P -DjLB+o0.2i+w2.2i/0+jBL -F+glightgrey+pthinner+s-4p/-6p/grey20@40 << EOF > $ps
# Modified legend from example 10
H 10p,Times-Roman My Title
C red
L - L dying
C yellow
L - L in trouble
C darkgreen
L - L vigorous
C blue
L - L developing
C purple
L - L institutional
N 1
S 0.1i c 0.15i p300/12 0.25p 0.3i This circle is hachured
S 0.1i e 0.15i yellow 0.25p 0.3i This ellipse is yellow
EOF
